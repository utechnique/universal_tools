//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_view.h"
#include "systems/render/engine/lighting/ve_forward_shading.h"
#include "systems/render/engine/policy/ve_render_mesh_instance_policy.h"
#include "systems/render/engine/ve_render_stencil_ref.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
ForwardShading::ForwardShading(Toolset& toolset,
                               ut::uint32 ibl_mip_count) : tools(toolset)
                                                         , lightpass(CreateLightPass())
                                                         , ibl_shader(CreateMeshInstIblShader(ibl_mip_count))
                                                         , light_shader(CreateMeshInstLightPassShader())
                                                         , emissive_shader(CreateMeshInstEmissivePassShader())
                                                         , ibl_pass_pipeline(CreateMeshInstIblPassPipelinePermutations())
                                                         , light_pass_pipeline(CreateMeshInstLightPassPipelinePermutations())
                                                         , emissive_pass_pipeline(CreateMeshInstEmissivePassPipelinePermutations())
{
	ConnectDescriptors();
}

// Creates forward shading (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param light_buffer - reference to the light buffer.
//    @param is_cube - 'true' to create as a cubemap.
//    @return - a new ForwardShading::ViewData object or error if failed.
ut::Result<ForwardShading::ViewData, ut::Error> ForwardShading::CreateViewData(Target& depth_stencil,
                                                                               Target& light_buffer,
                                                                               bool is_cube)
{
	const ut::uint32 face_count = is_cube ? 6 : 1;
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());

	// light pass framebuffer
	ut::Array<Framebuffer> light_framebuffer;
	for (ut::uint32 face_id = 0; face_id < face_count; face_id++)
	{
		ut::Array<Framebuffer::Attachment> color_targets;
		color_targets.Add(Framebuffer::Attachment(light_buffer, face_id));
		ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(lightpass,
		                                                                                ut::Move(color_targets),
		                                                                                Framebuffer::Attachment(depth_stencil, face_id));
		if (!framebuffer)
		{
			return ut::MakeError(framebuffer.MoveAlt());
		}
		light_framebuffer.Add(framebuffer.Move());
	}

	// secondary command buffers
	const ut::uint32 secondary_buffer_count = thread_count * (is_cube ? 6 : 1);
	ut::Array<CmdBuffer> lightpass_cmd_buffers;
	for (ut::uint32 i = 0; i < secondary_buffer_count; i++)
	{
		CmdBuffer::Info cmd_buffer_info;
		cmd_buffer_info.usage = CmdBuffer::Usage::dynamic_inside_render_pass;
		cmd_buffer_info.level = CmdBuffer::Level::secondary;

		ut::Result<CmdBuffer, ut::Error> cmd_buffer = tools.device.CreateCmdBuffer(cmd_buffer_info);
		if (!cmd_buffer)
		{
			throw ut::Error(cmd_buffer.MoveAlt());
		}

		lightpass_cmd_buffers.Add(cmd_buffer.Move());
	}

	ForwardShading::ViewData data =
	{
		ut::Move(light_framebuffer),
		ut::Move(lightpass_cmd_buffers),
	};

	return ut::Move(data);
}

// Renders units directly to the light buffer.
void ForwardShading::Draw(Context& context,
                          ForwardShading::ViewData& data,
                          Buffer& view_uniform_buffer,
                          const ut::Vector<3>& view_position,
                          Batcher& batcher,
                          Light::Sources& lights,
                          ut::Optional<Image&> ibl_cubemap,
                          Image::Cube::Face cubeface)
{
	// get the number of available threads
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	UT_ASSERT(thread_count == light_pass_desc_set.Count());

	// perform z-sorting
	SortTransparentDrawCalls(view_position, batcher.draw_calls);
	const ut::uint32 sorted_dc_count = static_cast<ut::uint32>(z_sorted_dc_indices.Count());

	// initialize secondary buffers for a parallel work
	const ut::uint32 dc_per_thread = sorted_dc_count / thread_count; // drawcalls per thread
	secondary_buffer_cache.Reset();
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		secondary_buffer_cache.Add(data.lightpass_cmd[static_cast<ut::uint32>(cubeface) * thread_count + i]);
		tools.device.ResetCmdBuffer(secondary_buffer_cache.GetLast().Get());
	}

	// begin a render pass
	Framebuffer& framebuffer = data.light_framebuffer[static_cast<ut::uint32>(cubeface)];
	const Framebuffer::Info& fb_info = framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(lightpass, framebuffer, render_area, ut::Color<4>(0), 1.0f, 0, true);

	// render all mesh instances in different threads
	ut::uint32 offset = 0; // offset from the first drawcall in the batcher
	for (ut::uint32 thread_id = 0; thread_id < thread_count; thread_id++)
	{
		ut::uint32 count = dc_per_thread + (thread_id == 0 ? (sorted_dc_count % thread_count) : 0);
		auto record_commands = [&, thread_id, offset, count](Context& deferred_context)
		{
			RenderTransparentMeshInstancesJob(deferred_context,
			                                  data,
			                                  lights,
			                                  view_uniform_buffer,
			                                  batcher,
			                                  ibl_cubemap,
			                                  thread_id,
			                                  offset,
			                                  count);
		};

		auto job = [&, thread_id, record_commands]()
		{
			tools.device.Record(secondary_buffer_cache[thread_id],
			                    record_commands,
			                    lightpass,
			                    framebuffer);
		};

		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void()> >(job));

		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// finish render pass
	context.ExecuteSecondaryBuffers(secondary_buffer_cache);
	context.EndRenderPass();
}

// Renders specified range of mesh instances.
void ForwardShading::RenderTransparentMeshInstancesJob(Context& context,
                                                       ForwardShading::ViewData& data,
                                                       Light::Sources& lights,
                                                       Buffer& view_uniform_buffer,
                                                       Batcher& batcher,
                                                       ut::Optional<Image&> ibl_cubemap,
                                                       ut::uint32 thread_id,
                                                       ut::uint32 offset,
                                                       ut::uint32 count)
{
	if (count == 0)
	{
		return;
	}

	const MeshInstRendering::IblPreset ibl_preset = ibl_cubemap ? MeshInstRendering::IblPreset::on :
	                                                              MeshInstRendering::IblPreset::off;
	const ut::uint32 last_element = offset + count - 1;
	for (ut::uint32 i = offset; i <= last_element; i++)
	{
		// skip opaque mesh instances
		const ut::uint32 sorted_id = z_sorted_dc_indices[i].first;
		MeshInstance::DrawCall& dc = batcher.draw_calls[sorted_id];
		Mesh::Subset& subset = dc.instance.mesh->subsets[dc.subset_id];
		Material& material = subset.material;
		const bool is_tansparent = material.alpha == Material::Alpha::transparent;

		constexpr ut::uint32 front_culling = 0;
		constexpr ut::uint32 back_culling = 1;
		constexpr ut::uint32 cull_mode_count = 2;
		ut::uint32 cull_mode_id = material.double_sided ? front_culling : back_culling;
		for (; cull_mode_id < cull_mode_count; cull_mode_id++)
		{
			bool first_pass = true;

			const MeshInstRendering::CullMode cull_mode = cull_mode_id == front_culling ?
			                                              MeshInstRendering::CullMode::front :
			                                              MeshInstRendering::CullMode::back;

			// ibl reflections pass
			if (ibl_cubemap)
			{
				const MeshInstRendering::AlphaMode first_pass_alpha_mode = is_tansparent ?
				                                                           MeshInstRendering::AlphaMode::blend :
				                                                           MeshInstRendering::AlphaMode::overwrite;
				const MeshInstRendering::AlphaMode ibl_alpha_mode = first_pass ?
				                                                    first_pass_alpha_mode :
				                                                    MeshInstRendering::AlphaMode::add;
				RenderTransparentMeshInstanceIbl(context,
				                                 data,
				                                 ibl_cubemap.Get(),
				                                 view_uniform_buffer,
				                                 batcher,
				                                 cull_mode,
				                                 ibl_alpha_mode,
				                                 sorted_id,
				                                 thread_id);
				first_pass = false;
			}

			// light pass
			const MeshInstRendering::AlphaMode first_pass_alpha_mode = is_tansparent ?
			                                                           MeshInstRendering::AlphaMode::blend :
			                                                           MeshInstRendering::AlphaMode::overwrite;
			const MeshInstRendering::AlphaMode lightpass_alpha_mode = first_pass ?
			                                                          first_pass_alpha_mode :
			                                                          MeshInstRendering::AlphaMode::add;
			RenderTransparentMeshInstanceLights(context,
			                                    data,
			                                    lights,
			                                    view_uniform_buffer,
			                                    batcher,
			                                    ibl_preset,
			                                    cull_mode,
			                                    lightpass_alpha_mode,
			                                    sorted_id,
			                                    thread_id);

			// emissive
			RenderTransparentMeshInstanceEmissive(context,
			                                      data,
			                                      view_uniform_buffer,
			                                      batcher,
			                                      cull_mode,
			                                      lightpass_alpha_mode,
			                                      sorted_id,
			                                      thread_id);
		}
	}
}

// Applies direct lighting to the specified mesh instance.
void ForwardShading::RenderTransparentMeshInstanceIbl(Context& context,
                                                      ForwardShading::ViewData& data,
                                                      Image& ibl_cubemap,
                                                      Buffer& view_uniform_buffer,
                                                      Batcher& batcher,
                                                      MeshInstRendering::CullMode cull_mode,
                                                      MeshInstRendering::AlphaMode alpha_mode,
                                                      ut::uint32 drawcall_id,
                                                      ut::uint32 thread_id)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	const ut::uint32 batch_size = batcher.GetBatchSize();
	ut::Array<MeshInstance::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	Buffer& instance_buffer = batcher.instance_buffer;

	// extract material
	MeshInstance::DrawCall& dc = batcher.draw_calls[drawcall_id];
	Mesh& mesh = dc.instance.mesh.Get();
	Mesh::Subset& subset = mesh.subsets[dc.subset_id];
	Material& material = subset.material;
	const MeshInstRendering::AlphaTest alpha_test = material.alpha == Material::Alpha::masked ?
	                                                MeshInstRendering::AlphaTest::on :
	                                                MeshInstRendering::AlphaTest::off;

	// get stencil mode
	const MeshInstRendering::StencilMode stencil_mode = dc.instance.highlighted ?
	                                                    MeshInstRendering::StencilMode::highlighted :
	                                                    MeshInstRendering::StencilMode::none;

	// calculate batch id
	const ut::uint32 batch_id = drawcall_id / batch_size;
	MeshInstance::Batch& batch = batches[batch_id];

	// get pipeline state
	UT_ASSERT(subset.vertex_buffer);
	const Mesh::VertexFormat vertex_format = subset.vertex_buffer->format;
	const Mesh::PolygonMode polygon_mode = subset.polygon_mode;
	const size_t pipeline_state_id = MeshInstRendering::IblPass::PipelineGrid::GetId(static_cast<size_t>(vertex_format),
	                                                                                 static_cast<size_t>(alpha_test),
	                                                                                 static_cast<size_t>(alpha_mode),
	                                                                                 static_cast<size_t>(cull_mode),
	                                                                                 static_cast<size_t>(stencil_mode),
	                                                                                 static_cast<size_t>(polygon_mode));

	// bind uniforms
	MeshInstRendering::IblPass::Descriptors& desc_set = ibl_pass_desc_set[thread_id];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
	desc_set.transform_ub.BindUniformBuffer(batch.transform);
	desc_set.material_ub.BindUniformBuffer(batch.material);
	desc_set.base_color.BindImage(material.base_color.Get());
	desc_set.normal.BindImage(material.normal.Get());
	desc_set.metallic_roughness.BindImage(material.metallic_roughness.Get());
	desc_set.ibl_cubemap.BindImage(ibl_cubemap);

	// draw
	DrawMesh(context,
	         subset,
	         ibl_pass_pipeline[pipeline_state_id],
	         desc_set,
	         instance_buffer,
	         drawcall_id % batch_size);
}

// Applies direct lighting to the specified mesh instance.
void ForwardShading::RenderTransparentMeshInstanceLights(Context& context,
                                                         ForwardShading::ViewData& data,
                                                         Light::Sources& lights,
                                                         Buffer& view_uniform_buffer,
                                                         Batcher& batcher,
                                                         MeshInstRendering::IblPreset ibl_preset,
                                                         MeshInstRendering::CullMode cull_mode,
                                                         MeshInstRendering::AlphaMode alpha_mode,
                                                         ut::uint32 drawcall_id,
                                                         ut::uint32 thread_id)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	const ut::uint32 batch_size = batcher.GetBatchSize();
	ut::Array<MeshInstance::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	Buffer& instance_buffer = batcher.instance_buffer;

	// extract material
	MeshInstance::DrawCall& dc = batcher.draw_calls[drawcall_id];
	Mesh& mesh = dc.instance.mesh.Get();
	Mesh::Subset& subset = mesh.subsets[dc.subset_id];
	Material& material = subset.material;
	const MeshInstRendering::AlphaTest alpha_test = material.alpha == Material::Alpha::masked ?
	                                                MeshInstRendering::AlphaTest::on :
	                                                MeshInstRendering::AlphaTest::off;

	// get stencil mode
	const MeshInstRendering::StencilMode stencil_mode = dc.instance.highlighted ?
	                                                    MeshInstRendering::StencilMode::highlighted :
	                                                    MeshInstRendering::StencilMode::none;

	// accumulate light from all sources
	const size_t ambient_count = lights.ambient.Count();
	const size_t directional_count = lights.directional.Count();
	const size_t point_count = lights.point.Count();
	const size_t spot_count = lights.spot.Count();
	const size_t light_count = ambient_count + directional_count + point_count + spot_count;
	for (size_t light_id = 0; light_id < light_count; light_id++)
	{
		// figure out source type
		const Light::SourceType light_type = light_id < ambient_count ? Light::SourceType::ambient :
		                                     light_id < (ambient_count + directional_count) ? Light::SourceType::directional :
		                                     light_id < (ambient_count + directional_count + point_count) ?
		                                     Light::SourceType::point : Light::SourceType::spot;

		// extract light data
		ut::Optional<Buffer&> light_ub;
		if (light_type == Light::SourceType::ambient)
		{
			AmbientLight& light = lights.ambient[light_id];
			AmbientLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::SourceType::directional)
		{
			DirectionalLight& light = lights.directional[light_id - ambient_count];
			DirectionalLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::SourceType::point)
		{
			PointLight& light = lights.point[light_id - ambient_count - directional_count];
			PointLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::SourceType::spot)
		{
			SpotLight& light = lights.spot[light_id - ambient_count - directional_count - point_count];
			SpotLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else
		{
			UT_ASSERT(false);
		}

		// blending mode is used only for the first pass
		if (light_id > 0)
		{
			alpha_mode = MeshInstRendering::AlphaMode::add;
		}

		// calculate batch id
		const ut::uint32 batch_id = drawcall_id / batch_size;
		MeshInstance::Batch& batch = batches[batch_id];

		// bind pipeline state
		UT_ASSERT(subset.vertex_buffer);
		const Mesh::VertexFormat vertex_format = subset.vertex_buffer->format;
		const Mesh::PolygonMode polygon_mode = subset.polygon_mode;
		const size_t pipeline_state_id = MeshInstRendering::LightPass::PipelineGrid::GetId(static_cast<size_t>(vertex_format),
		                                                                                   static_cast<size_t>(alpha_test),
		                                                                                   static_cast<size_t>(alpha_mode),
		                                                                                   static_cast<size_t>(cull_mode),
		                                                                                   static_cast<size_t>(stencil_mode),
		                                                                                   static_cast<size_t>(polygon_mode),
		                                                                                   static_cast<size_t>(ibl_preset),
		                                                                                   static_cast<size_t>(light_type));

		// deduce what descriptor set to use according to the type of the light
		MeshInstRendering::MeshDescriptors& ambient_desc_set = ambient_pass_desc_set[static_cast<size_t>(ibl_preset)][thread_id];
		MeshInstRendering::MeshDescriptors& direct_desc_set = light_pass_desc_set[thread_id];
		MeshInstRendering::MeshDescriptors& desc_set = light_type == Light::SourceType::ambient ?
		                                               ambient_desc_set : direct_desc_set;

		// bind common uniforms
		desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
		desc_set.light_ub.BindUniformBuffer(light_ub.Get());
		desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
		desc_set.transform_ub.BindUniformBuffer(batch.transform);
		desc_set.material_ub.BindUniformBuffer(batch.material);
		desc_set.base_color.BindImage(material.base_color.Get());
		if (light_type != Light::SourceType::ambient)
		{
			desc_set.normal.BindImage(material.normal.Get());
		}
		if (light_type != Light::SourceType::ambient ||
		    ibl_preset == MeshInstRendering::IblPreset::on)
		{
			desc_set.metallic_roughness.BindImage(material.metallic_roughness.Get());
		}
		if (light_type == Light::SourceType::ambient)
		{
			desc_set.occlusion.BindImage(material.occlusion.Get());
		}

		// bind index buffer and draw
		DrawMesh(context,
		         subset,
		         light_pass_pipeline[pipeline_state_id],
		         desc_set,
		         instance_buffer,
		         drawcall_id % batch_size);
	}
}

// Applies emissive lighting to the specified mesh instance.
void ForwardShading::RenderTransparentMeshInstanceEmissive(Context& context,
                                                           ForwardShading::ViewData& data,
                                                           Buffer& view_uniform_buffer,
                                                           Batcher& batcher,
                                                           MeshInstRendering::CullMode cull_mode,
                                                           MeshInstRendering::AlphaMode alpha_mode,
                                                           ut::uint32 drawcall_id,
                                                           ut::uint32 thread_id)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	const ut::uint32 batch_size = batcher.GetBatchSize();
	ut::Array<MeshInstance::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	Buffer& instance_buffer = batcher.instance_buffer;

	// extract material
	MeshInstance::DrawCall& dc = batcher.draw_calls[drawcall_id];
	Mesh& mesh = dc.instance.mesh.Get();
	Mesh::Subset& subset = mesh.subsets[dc.subset_id];
	Material& material = subset.material;
	const MeshInstRendering::AlphaTest alpha_test = material.alpha == Material::Alpha::masked ?
	                                                MeshInstRendering::AlphaTest::on :
	                                                MeshInstRendering::AlphaTest::off;

	// get stencil mode
	const MeshInstRendering::StencilMode stencil_mode = dc.instance.highlighted ?
	                                                    MeshInstRendering::StencilMode::highlighted :
	                                                    MeshInstRendering::StencilMode::none;

	// calculate batch id
	const ut::uint32 batch_id = drawcall_id / batch_size;
	MeshInstance::Batch& batch = batches[batch_id];

	// get pipeline state
	UT_ASSERT(subset.vertex_buffer);
	const Mesh::VertexFormat vertex_format = subset.vertex_buffer->format;
	const Mesh::PolygonMode polygon_mode = subset.polygon_mode;
	const size_t pipeline_state_id = MeshInstRendering::EmissivePass::PipelineGrid::GetId(static_cast<size_t>(vertex_format),
	                                                                                      static_cast<size_t>(alpha_test),
	                                                                                      static_cast<size_t>(alpha_mode),
	                                                                                      static_cast<size_t>(cull_mode),
	                                                                                      static_cast<size_t>(stencil_mode),
	                                                                                      static_cast<size_t>(polygon_mode));

	// bind uniforms
	MeshInstRendering::EmissivePass::Descriptors& desc_set =
		emissive_pass_desc_set[static_cast<size_t>(alpha_test)][thread_id];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
	desc_set.transform_ub.BindUniformBuffer(batch.transform);
	desc_set.material_ub.BindUniformBuffer(batch.material);
	desc_set.emissive.BindImage(material.emissive.Get());
	if (alpha_test == MeshInstRendering::AlphaTest::on)
	{
		desc_set.base_color.BindImage(material.base_color.Get());
	}

	// draw
	DrawMesh(context,
	         subset,
	         emissive_pass_pipeline[pipeline_state_id],
	         desc_set,
	         instance_buffer,
	         drawcall_id % batch_size);
}

// Renders provided mesh.
void ForwardShading::DrawMesh(Context& context,
                              Mesh::Subset& mesh_subset,
                              PipelineState& pipeline,
                              DescriptorSet& desc_set,
                              Buffer& instance_buffer,
                              ut::uint32 instance_offset)
{
	context.BindPipelineState(pipeline);
	context.BindDescriptorSet(desc_set);
	context.BindVertexAndInstanceBuffer(mesh_subset.vertex_buffer.GetRef(), 0,
	                                    instance_buffer, 0);
	if (mesh_subset.index_buffer)
	{
		context.BindIndexBuffer(mesh_subset.index_buffer.GetRef(), 0,
		                        mesh_subset.index_buffer->format);
		context.DrawIndexedInstanced(mesh_subset.count,
		                             1,
		                             mesh_subset.offset,
		                             0,
		                             instance_offset);
	}
	else
	{
		context.DrawInstanced(mesh_subset.count, // index count here means vertex count
		                      1,
		                      mesh_subset.offset, // index offset here means vertex offset
		                      instance_offset);
	}
}

// Performes Z-sotring for transparent objects and stores the result in
// @z_sorted_dc_indices array.
void ForwardShading::SortTransparentDrawCalls(const ut::Vector<3>& view_position,
                                              ut::Array<MeshInstance::DrawCall>& draw_list)
{
	z_sorted_dc_indices.Reset();
	const ut::uint32 dc_count = static_cast<ut::uint32>(draw_list.Count());
	for (ut::uint32 dc_index = 0; dc_index < dc_count; dc_index++)
	{
		// skip opaque and unlit draw calls
		const MeshInstance::DrawCall& dc = draw_list[dc_index];
		const Mesh::Subset& subset = dc.instance.mesh->subsets[dc.subset_id];
		const Material& material = subset.material;
		const bool skip_opaque = !dc.instance.force_forward_renderer &&
		                         material.alpha != Material::Alpha::transparent;
		const bool skip = material.unlit || skip_opaque;
		if (skip)
		{
			continue;
		}

		// calculate distance
		const ut::Vector<3> direction = dc.instance.world_transform.translation - view_position;
		const float distance = direction.Length();
		ut::Pair<ut::uint32, float> element_to_sort(dc_index, distance);

		// insert the view using the 'bubble' principle to retain sorting
		bool inserted = false;
		const size_t sorted_dc_count = z_sorted_dc_indices.Count();
		for (size_t sorted_dc_index = 0; sorted_dc_index < sorted_dc_count; sorted_dc_index++)
		{
			const ut::Pair<ut::uint32, float>& sorted_id = z_sorted_dc_indices[sorted_dc_index];
			if (distance >= sorted_id.second)
			{
				z_sorted_dc_indices.Insert(sorted_dc_index, ut::Move(element_to_sort));
				inserted = true;
				break;
			}
		}

		if (!inserted)
		{
			z_sorted_dc_indices.Add(ut::Move(element_to_sort));
		}
	}
}

// Creates a render pass for the shading techniques.
RenderPass ForwardShading::CreateLightPass()
{
	RenderTargetSlot depth_slot(tools.formats.depth_stencil,
	                            RenderTargetSlot::LoadOperation::extract,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	RenderTargetSlot color_slot(tools.formats.light_buffer,
	                            RenderTargetSlot::LoadOperation::extract,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot).MoveOrThrow();
}

// Creates a shader for the ibl pass.
ut::Array<BoundShader> ForwardShading::CreateMeshInstIblShader(ut::uint32 ibl_mip_count)
{
	// calculate multithreading data
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t shader_permutation_count = MeshInstRendering::IblPass::ShaderGrid::size;
	const size_t shader_permutations_per_thread = shader_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<BoundShader> > temp_shader_cache(shader_permutation_count);

	// multithreading job
	auto load_shader_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = MeshInstRendering::IblPass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::vertex_format_column>(i);
			const size_t alpha_test = MeshInstRendering::IblPass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::alpha_test_column>(i);

			Shader::Macros macros;
			Shader::MacroDefinition macro;
			ut::String shader_name_suffix;

			// enable instancing
			macro.name = "INSTANCING";
			macro.value = "1";
			macros.Add(macro);

			// ibl pass
			macro.name = "IBL_PASS";
			macro.value = "1";
			macros.Add(macro);
			macro.name = "IBL";
			macro.value = "1";
			macros.Add(macro);

			// batch size
			macro.name = "BATCH_SIZE";
			macro.value = ut::Print(Batcher::CalculateBatchSize(tools.device));
			macros.Add(macro);

			// mip count
			macro.name = "IBL_MIP_COUNT";
			macro.value = ut::Print(ibl_mip_count);
			macros.Add(ut::Move(macro));

			// vertex traits
			macros += Mesh::GenerateVertexMacros(static_cast<Mesh::VertexFormat>(vertex_format), Mesh::Instancing::on);
			shader_name_suffix += ut::String("_vf") + ut::Print(vertex_format);

			// alpha test
			const bool alpha_test_enabled = alpha_test == static_cast<size_t>(MeshInstRendering::AlphaTest::on);
			macro.name = "ALPHA_TEST";
			macro.value = alpha_test_enabled ? "1" : "0";
			macros.Add(macro);
			shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

			ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::Stage::vertex,
			                                                            ut::String("forward_mesh_ibl_vs") + shader_name_suffix,
			                                                            "VS",
			                                                            "mesh.hlsl",
			                                                            macros);

			ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel,
			                                                            ut::String("forward_mesh_ibl_ps") + shader_name_suffix,
			                                                            "PS",
			                                                            "mesh.hlsl",
			                                                            macros);

			temp_shader_cache[i] = ut::MakeUnique<BoundShader>(vs.MoveOrThrow(), ps.MoveOrThrow());
		}
	};

	// compile shaders in multiple threads
	for (ut::uint32 thread_id = 0, offset = 0; thread_id < thread_count; thread_id++)
	{
		const ut::uint32 count = static_cast<ut::uint32>(shader_permutations_per_thread) +
			(thread_id == 0 ? (static_cast<ut::uint32>(shader_permutation_count) % thread_count) : 0);
		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void(ut::uint32,
		                                                      ut::uint32)> >(load_shader_job,
		                                                                     offset, count));
		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// get rid of unique ptr containers
	ut::Array<BoundShader> shaders;
	for (ut::uint32 i = 0; i < shader_permutation_count; i++)
	{
		if (!shaders.Add(ut::Move(temp_shader_cache[i].GetRef())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
	return shaders;
}

// Creates a shader for the lighting pass.
ut::Array<BoundShader> ForwardShading::CreateMeshInstLightPassShader()
{
	// calculate multithreading data
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t shader_permutation_count = MeshInstRendering::LightPass::ShaderGrid::size;
	const size_t shader_permutations_per_thread = shader_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<BoundShader> > temp_shader_cache(shader_permutation_count);

	// multithreading job
	auto load_shader_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = MeshInstRendering::LightPass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::vertex_format_column>(i);
			const size_t alpha_test = MeshInstRendering::LightPass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::alpha_test_column>(i);
			const size_t ibl_preset = MeshInstRendering::LightPass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::ibl_column>(i);
			const size_t source_type = MeshInstRendering::LightPass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::light_type_column>(i);

			Shader::Macros macros;
			Shader::MacroDefinition macro;
			ut::String shader_name_suffix;

			// enable instancing
			macro.name = "INSTANCING";
			macro.value = "1";
			macros.Add(macro);

			// light pass
			macro.name = "LIGHT_PASS";
			macro.value = "1";
			macros.Add(macro);

			// batch size
			macro.name = "BATCH_SIZE";
			macro.value = ut::Print(Batcher::CalculateBatchSize(tools.device));
			macros.Add(macro);

			// vertex traits
			macros += Mesh::GenerateVertexMacros(static_cast<Mesh::VertexFormat>(vertex_format), Mesh::Instancing::on);
			shader_name_suffix += ut::String("_vf") + ut::Print(vertex_format);

			// ibl preset
			const bool ibl_enabled = ibl_preset == static_cast<size_t>(MeshInstRendering::IblPreset::on);
			shader_name_suffix += ibl_enabled ? "_ibl" : "_noibl";
			macro.name = "IBL";
			macro.value = ibl_enabled ? "1" : "0";
			macros.Add(ut::Move(macro));

			// light type
			const char* light_type_str;
			switch (static_cast<Light::SourceType>(source_type))
			{
			case Light::SourceType::directional:
				light_type_str = "DIRECTIONAL_LIGHT";
				shader_name_suffix += "_directional";
				break;
			case Light::SourceType::point:
				light_type_str = "POINT_LIGHT";
				shader_name_suffix += "_point";
				break;
			case Light::SourceType::spot:
				light_type_str = "SPOT_LIGHT";
				shader_name_suffix += "_spot";
				break;
			case Light::SourceType::ambient:
				light_type_str = "AMBIENT_LIGHT";
				shader_name_suffix += "_ambient";
				break;
			default: throw ut::Error(ut::error::not_implemented);
			}
			macro.name = light_type_str;
			macro.value = "1";
			macros.Add(ut::Move(macro));

			// alpha test
			const bool alpha_test_enabled = alpha_test == static_cast<size_t>(MeshInstRendering::AlphaTest::on);
			macro.name = "ALPHA_TEST";
			macro.value = alpha_test_enabled ? "1" : "0";
			macros.Add(macro);
			shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

			ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::Stage::vertex,
																		ut::String("forward_mesh_lp_vs") + shader_name_suffix,
																		"VS",
																		"mesh.hlsl",
																		macros);

			ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel,
			                                                            ut::String("forward_mesh_lp_ps") + shader_name_suffix,
			                                                            "PS",
			                                                            "mesh.hlsl",
			                                                            macros);

			temp_shader_cache[i] = ut::MakeUnique<BoundShader>(vs.MoveOrThrow(), ps.MoveOrThrow());
		}
	};

	// compile shaders in multiple threads
	for (ut::uint32 thread_id = 0, offset = 0; thread_id < thread_count; thread_id++)
	{
		const ut::uint32 count = static_cast<ut::uint32>(shader_permutations_per_thread) +
			(thread_id == 0 ? (static_cast<ut::uint32>(shader_permutation_count) % thread_count) : 0);
		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void(ut::uint32,
		                                                      ut::uint32)> >(load_shader_job,
		                                                                     offset, count));
		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// get rid of unique ptr containers
	ut::Array<BoundShader> shaders;
	for (ut::uint32 i = 0; i < shader_permutation_count; i++)
	{
		if (!shaders.Add(ut::Move(temp_shader_cache[i].GetRef())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
	return shaders;
}

// Creates a shader for the emissive pass.
ut::Array<BoundShader> ForwardShading::CreateMeshInstEmissivePassShader()
{
	// calculate multithreading data
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t shader_permutation_count = MeshInstRendering::EmissivePass::ShaderGrid::size;
	const size_t shader_permutations_per_thread = shader_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<BoundShader> > temp_shader_cache(shader_permutation_count);

	// multithreading job
	auto load_shader_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = MeshInstRendering::EmissivePass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::vertex_format_column>(i);
			const size_t alpha_test = MeshInstRendering::EmissivePass::ShaderGrid::GetCoordinate<MeshInstRendering::ShaderGrid::alpha_test_column>(i);

			Shader::Macros macros;
			Shader::MacroDefinition macro;
			ut::String shader_name_suffix;

			// enable instancing
			macro.name = "INSTANCING";
			macro.value = "1";
			macros.Add(macro);

			// emissive pass
			macro.name = "EMISSIVE_PASS";
			macro.value = "1";
			macros.Add(macro);

			// batch size
			macro.name = "BATCH_SIZE";
			macro.value = ut::Print(Batcher::CalculateBatchSize(tools.device));
			macros.Add(macro);

			// vertex traits
			macros += Mesh::GenerateVertexMacros(static_cast<Mesh::VertexFormat>(vertex_format), Mesh::Instancing::on);
			shader_name_suffix += ut::String("_vf") + ut::Print(vertex_format);

			// alpha test
			const bool alpha_test_enabled = alpha_test == static_cast<size_t>(MeshInstRendering::AlphaTest::on);
			macro.name = "ALPHA_TEST";
			macro.value = alpha_test_enabled ? "1" : "0";
			macros.Add(macro);
			shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

			ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::Stage::vertex,
																		ut::String("forward_mesh_emissive_vs") + shader_name_suffix,
																		"VS",
																		"mesh.hlsl",
																		macros);

			ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel,
			                                                            ut::String("forward_mesh_emissive_ps") + shader_name_suffix,
			                                                            "PS",
			                                                            "mesh.hlsl",
			                                                            macros);

			temp_shader_cache[i] = ut::MakeUnique<BoundShader>(vs.MoveOrThrow(), ps.MoveOrThrow());
		}
	};

	// compile shaders in multiple threads
	for (ut::uint32 thread_id = 0, offset = 0; thread_id < thread_count; thread_id++)
	{
		const ut::uint32 count = static_cast<ut::uint32>(shader_permutations_per_thread) +
			(thread_id == 0 ? (static_cast<ut::uint32>(shader_permutation_count) % thread_count) : 0);
		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void(ut::uint32,
		                                                      ut::uint32)> >(load_shader_job,
		                                                                     offset, count));
		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// get rid of unique ptr containers
	ut::Array<BoundShader> shaders;
	for (ut::uint32 i = 0; i < shader_permutation_count; i++)
	{
		if (!shaders.Add(ut::Move(temp_shader_cache[i].GetRef())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
	return shaders;
}

// Creates a pipeline state to apply ibl reflections.
ut::Result<PipelineState, ut::Error> ForwardShading::CreateMeshInstIblPassPipeline(Mesh::VertexFormat vertex_format,
                                                                                   Mesh::PolygonMode polygon_mode,
                                                                                   MeshInstRendering::AlphaTest alpha_test,
                                                                                   MeshInstRendering::AlphaMode alpha_mode,
                                                                                   MeshInstRendering::CullMode cull_mode,
                                                                                   MeshInstRendering::StencilMode stencil_mode)
{
	PipelineState::Info info;
	const size_t shader_id = MeshInstRendering::IblPass::ShaderGrid::GetId(static_cast<size_t>(vertex_format),
	                                                                       static_cast<size_t>(alpha_test));
	UT_ASSERT(ibl_shader[shader_id].GetShader(Shader::Stage::vertex));
	UT_ASSERT(ibl_shader[shader_id].GetShader(Shader::Stage::pixel));

	const ut::uint32 stencil_mask = stencil_mode == MeshInstRendering::StencilMode::highlighted ?
	                                                static_cast<ut::uint32>(StencilReference::highlight) : 0x0;

	info.SetShader(Shader::Stage::vertex,
	               ibl_shader[shader_id].GetShader(Shader::Stage::vertex));
	info.SetShader(Shader::Stage::pixel,
	               ibl_shader[shader_id].GetShader(Shader::Stage::pixel));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format,
	                                                polygon_mode,
	                                                Mesh::Instancing::on);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = alpha_mode == MeshInstRendering::AlphaMode::overwrite;
	info.depth_stencil_state.depth_compare_op = compare::Operation::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.compare_mask = 0xffffffff;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xffffffff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = Mesh::GetRasterizerPolygonMode(polygon_mode);
	info.rasterization_state.cull_mode = cull_mode == MeshInstRendering::CullMode::back ?
	                                                  RasterizationState::CullMode::back :
	                                                  RasterizationState::CullMode::off;
	if (alpha_mode == MeshInstRendering::AlphaMode::blend)
	{
		Blending blending(true,
		                  Blending::Factor::one,
		                  Blending::Factor::inverted_src_alpha,
		                  Blending::Operation::add,
		                  Blending::Factor::one,
		                  Blending::Factor::one,
		                  Blending::Operation::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == MeshInstRendering::AlphaMode::add)
	{
		Blending blending(true,
		                  Blending::Factor::one,
		                  Blending::Factor::one,
		                  Blending::Operation::add,
		                  Blending::Factor::one,
		                  Blending::Factor::one,
		                  Blending::Operation::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == MeshInstRendering::AlphaMode::overwrite)
	{
		info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	}
	
	return tools.device.CreatePipelineState(ut::Move(info), lightpass);
}

// Creates a pipeline state to apply lighting.
ut::Result<PipelineState, ut::Error> ForwardShading::CreateMeshInstLightPassPipeline(Mesh::VertexFormat vertex_format,
                                                                                     Mesh::PolygonMode polygon_mode,
                                                                                     MeshInstRendering::AlphaTest alpha_test,
                                                                                     MeshInstRendering::AlphaMode alpha_mode,
                                                                                     MeshInstRendering::CullMode cull_mode,
                                                                                     MeshInstRendering::StencilMode stencil_mode,
                                                                                     MeshInstRendering::IblPreset ibl_preset,
                                                                                     Light::SourceType source_type)
{
	PipelineState::Info info;
	const size_t shader_id = MeshInstRendering::LightPass::ShaderGrid::GetId(static_cast<size_t>(vertex_format),
	                                                                         static_cast<size_t>(alpha_test),
	                                                                         static_cast<size_t>(ibl_preset),
	                                                                         static_cast<size_t>(source_type));
	UT_ASSERT(light_shader[shader_id].GetShader(Shader::Stage::vertex));
	UT_ASSERT(light_shader[shader_id].GetShader(Shader::Stage::pixel));

	const ut::uint32 stencil_mask = stencil_mode == MeshInstRendering::StencilMode::highlighted ?
	                                                static_cast<ut::uint32>(StencilReference::highlight) : 0x0;

	info.SetShader(Shader::Stage::vertex,
	               light_shader[shader_id].GetShader(Shader::Stage::vertex));
	info.SetShader(Shader::Stage::pixel,
	               light_shader[shader_id].GetShader(Shader::Stage::pixel));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format,
	                                                polygon_mode,
	                                                Mesh::Instancing::on);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = alpha_mode == MeshInstRendering::AlphaMode::overwrite;
	info.depth_stencil_state.depth_compare_op = compare::Operation::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.compare_mask = 0xffffffff;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xffffffff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = Mesh::GetRasterizerPolygonMode(polygon_mode);
	info.rasterization_state.cull_mode = cull_mode == MeshInstRendering::CullMode::back ?
	                                                  RasterizationState::CullMode::back :
	                                                  RasterizationState::CullMode::off;
	if (alpha_mode == MeshInstRendering::AlphaMode::blend)
	{
		Blending blending(true,
		                  Blending::Factor::src_alpha,
		                  Blending::Factor::inverted_src_alpha,
		                  Blending::Operation::add,
		                  Blending::Factor::one,
		                  Blending::Factor::one,
		                  Blending::Operation::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == MeshInstRendering::AlphaMode::add)
	{
		Blending blending(true,
		                  Blending::Factor::src_alpha,
		                  Blending::Factor::one,
		                  Blending::Operation::add,
		                  Blending::Factor::one,
		                  Blending::Factor::one,
		                  Blending::Operation::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == MeshInstRendering::AlphaMode::overwrite)
	{
		info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	}
	
	return tools.device.CreatePipelineState(ut::Move(info), lightpass);
}

// Creates a pipeline state to apply emissive color.
ut::Result<PipelineState, ut::Error> ForwardShading::CreateMeshInstEmissivePassPipeline(Mesh::VertexFormat vertex_format,
                                                                                        Mesh::PolygonMode polygon_mode,
                                                                                        MeshInstRendering::AlphaTest alpha_test,
                                                                                        MeshInstRendering::AlphaMode alpha_mode,
                                                                                        MeshInstRendering::CullMode cull_mode,
                                                                                        MeshInstRendering::StencilMode stencil_mode)
{
	PipelineState::Info info;
	const size_t shader_id = MeshInstRendering::EmissivePass::ShaderGrid::GetId(static_cast<size_t>(vertex_format),
	                                                                            static_cast<size_t>(alpha_test));
	UT_ASSERT(emissive_shader[shader_id].GetShader(Shader::Stage::vertex));
	UT_ASSERT(emissive_shader[shader_id].GetShader(Shader::Stage::pixel));

	const ut::uint32 stencil_mask = stencil_mode == MeshInstRendering::StencilMode::highlighted ?
	                                                static_cast<ut::uint32>(StencilReference::highlight) : 0x0;

	info.SetShader(Shader::Stage::vertex,
	               emissive_shader[shader_id].GetShader(Shader::Stage::vertex));
	info.SetShader(Shader::Stage::pixel,
	               emissive_shader[shader_id].GetShader(Shader::Stage::pixel));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format,
	                                                polygon_mode,
	                                                Mesh::Instancing::on);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.compare_mask = 0xffffffff;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xffffffff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = Mesh::GetRasterizerPolygonMode(polygon_mode);
	info.rasterization_state.cull_mode = cull_mode == MeshInstRendering::CullMode::back ?
	                                                  RasterizationState::CullMode::back :
	                                                  RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateAdditiveBlending());
	
	return tools.device.CreatePipelineState(ut::Move(info), lightpass);
}

// Creates all possible ibl pipeline state permutations for a mesh instance.
ut::Array<PipelineState> ForwardShading::CreateMeshInstIblPassPipelinePermutations()
{
	// calculate multithreading data to create ibl pass pipeline states
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t ibl_permutation_count = MeshInstRendering::IblPass::PipelineGrid::size;
	const size_t ibl_permutations_per_thread = ibl_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<PipelineState> > ibl_pipeline_mt_cache(ibl_permutation_count);

	// multithreading job to create iblpass pipeline
	auto mesh_inst_ibl_pipeline_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = MeshInstRendering::IblPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::vertex_format_column>(i);
			const size_t alpha_test = MeshInstRendering::IblPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::alpha_test_column>(i);
			const size_t alpha_mode = MeshInstRendering::IblPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::alpha_mode_column>(i);
			const size_t cull_mode = MeshInstRendering::IblPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::cull_mode_column>(i);
			const size_t stencil_mode = MeshInstRendering::IblPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::stencil_mode_column>(i);
			const size_t polygon_mode = MeshInstRendering::IblPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::polygon_mode_column>(i);

			ut::Result<PipelineState, ut::Error> pipeline = CreateMeshInstIblPassPipeline(static_cast<Mesh::VertexFormat>(vertex_format),
			                                                                              static_cast<Mesh::PolygonMode>(polygon_mode),
			                                                                              static_cast<MeshInstRendering::AlphaTest>(alpha_test),
			                                                                              static_cast<MeshInstRendering::AlphaMode>(alpha_mode),
			                                                                              static_cast<MeshInstRendering::CullMode>(cull_mode),
			                                                                              static_cast<MeshInstRendering::StencilMode>(stencil_mode));
			if (!pipeline)
			{
				throw ut::MakeError(pipeline.MoveAlt());
			}

			ibl_pipeline_mt_cache[i] = ut::MakeUnique<PipelineState>(ut::Move(pipeline.Move()));
		}
	};

	// create light pass pipeline in multiple threads
	for (ut::uint32 thread_id = 0, offset = 0; thread_id < thread_count; thread_id++)
	{
		const ut::uint32 count = static_cast<ut::uint32>(ibl_permutations_per_thread) +
			(thread_id == 0 ? (static_cast<ut::uint32>(ibl_permutation_count) % thread_count) : 0);
		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void(ut::uint32,
		                                                      ut::uint32)> >(mesh_inst_ibl_pipeline_job,
		                                                      offset, count));
		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	ut::Array<PipelineState> pipeline_permutations;
	for (ut::uint32 i = 0; i < ibl_permutation_count; i++)
	{
		if (!pipeline_permutations.Add(ut::Move(ibl_pipeline_mt_cache[i].GetRef())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return pipeline_permutations;
}

// Creates all possible lighting pipeline state permutations for a mesh instance.
ut::Array<PipelineState> ForwardShading::CreateMeshInstLightPassPipelinePermutations()
{
	// calculate multithreading data to create light pass pipeline states
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t light_permutation_count = MeshInstRendering::LightPass::PipelineGrid::size;
	const size_t light_permutations_per_thread = light_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<PipelineState> > lightpass_pipeline_mt_cache(light_permutation_count);

	// multithreading job to create lightpass pipeline
	auto mesh_inst_lp_pipeline_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::vertex_format_column>(i);
			const size_t alpha_test = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::alpha_test_column>(i);
			const size_t alpha_mode = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::alpha_mode_column>(i);
			const size_t cull_mode = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::cull_mode_column>(i);
			const size_t stencil_mode = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::stencil_mode_column>(i);
			const size_t polygon_mode = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::polygon_mode_column>(i);
			const size_t ibl_preset = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::ibl_column>(i);
			const size_t source_type = MeshInstRendering::LightPass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::light_type_column>(i);

			ut::Result<PipelineState, ut::Error> pipeline = CreateMeshInstLightPassPipeline(static_cast<Mesh::VertexFormat>(vertex_format),
			                                                                                static_cast<Mesh::PolygonMode>(polygon_mode),
			                                                                                static_cast<MeshInstRendering::AlphaTest>(alpha_test),
			                                                                                static_cast<MeshInstRendering::AlphaMode>(alpha_mode),
			                                                                                static_cast<MeshInstRendering::CullMode>(cull_mode),
			                                                                                static_cast<MeshInstRendering::StencilMode>(stencil_mode),
			                                                                                static_cast<MeshInstRendering::IblPreset>(ibl_preset),
			                                                                                static_cast<Light::SourceType>(source_type));
			if (!pipeline)
			{
				throw ut::Error(pipeline.MoveAlt());
			}

			lightpass_pipeline_mt_cache[i] = ut::MakeUnique<PipelineState>(ut::Move(pipeline.Move()));
		}
	};

	// create light pass pipeline in multiple threads
	for (ut::uint32 thread_id = 0, offset = 0; thread_id < thread_count; thread_id++)
	{
		const ut::uint32 count = static_cast<ut::uint32>(light_permutations_per_thread) +
			(thread_id == 0 ? (static_cast<ut::uint32>(light_permutation_count) % thread_count) : 0);
		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void(ut::uint32,
		                                                      ut::uint32)> >(mesh_inst_lp_pipeline_job,
		                                                                     offset, count));
		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// get rid of unique ptr containers
	ut::Array<PipelineState> pipeline_permutations;
	for (ut::uint32 i = 0; i < light_permutation_count; i++)
	{
		if (!pipeline_permutations.Add(ut::Move(lightpass_pipeline_mt_cache[i].GetRef())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return pipeline_permutations;
}

// Creates all possible emissive pipeline state permutations for a mesh instance.
ut::Array<PipelineState> ForwardShading::CreateMeshInstEmissivePassPipelinePermutations()
{
		// calculate multithreading data to create emissive pass pipeline states
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t emissive_permutation_count = MeshInstRendering::EmissivePass::PipelineGrid::size;
	const size_t emissive_permutations_per_thread = emissive_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<PipelineState> > emissive_pass_pipeline_mt_cache(emissive_permutation_count);

	// multithreading job to create emissive pass pipeline
	auto mesh_inst_lp_pipeline_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = MeshInstRendering::EmissivePass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::vertex_format_column>(i);
			const size_t alpha_test = MeshInstRendering::EmissivePass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::alpha_test_column>(i);
			const size_t alpha_mode = MeshInstRendering::EmissivePass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::alpha_mode_column>(i);
			const size_t cull_mode = MeshInstRendering::EmissivePass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::cull_mode_column>(i);
			const size_t stencil_mode = MeshInstRendering::EmissivePass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::stencil_mode_column>(i);
			const size_t polygon_mode = MeshInstRendering::EmissivePass::PipelineGrid::GetCoordinate<MeshInstRendering::PipelineGrid::polygon_mode_column>(i);

			ut::Result<PipelineState, ut::Error> pipeline = CreateMeshInstEmissivePassPipeline(static_cast<Mesh::VertexFormat>(vertex_format),
			                                                                                   static_cast<Mesh::PolygonMode>(polygon_mode),
			                                                                                   static_cast<MeshInstRendering::AlphaTest>(alpha_test),
			                                                                                   static_cast<MeshInstRendering::AlphaMode>(alpha_mode),
			                                                                                   static_cast<MeshInstRendering::CullMode>(cull_mode),
			                                                                                   static_cast<MeshInstRendering::StencilMode>(stencil_mode));
			if (!pipeline)
			{
				throw ut::Error(pipeline.MoveAlt());
			}

			emissive_pass_pipeline_mt_cache[i] = ut::MakeUnique<PipelineState>(ut::Move(pipeline.Move()));
		}
	};

	// create light pass pipeline in multiple threads
	for (ut::uint32 thread_id = 0, offset = 0; thread_id < thread_count; thread_id++)
	{
		const ut::uint32 count = static_cast<ut::uint32>(emissive_permutations_per_thread) +
			(thread_id == 0 ? (static_cast<ut::uint32>(emissive_permutation_count) % thread_count) : 0);
		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void(ut::uint32,
		                                                      ut::uint32)> >(mesh_inst_lp_pipeline_job,
		                                                                     offset, count));
		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// get rid of unique ptr containers
	ut::Array<PipelineState> pipeline_permutations;
	for (ut::uint32 i = 0; i < emissive_permutation_count; i++)
	{
		if (!pipeline_permutations.Add(ut::Move(emissive_pass_pipeline_mt_cache[i].GetRef())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return pipeline_permutations;
}

// Connects all descriptor sets to the corresponding shaders.
void ForwardShading::ConnectDescriptors()
{
	UT_ASSERT(light_shader.Count() != 0);
	UT_ASSERT(ibl_shader.Count() != 0);

	// get a mesh instance shader for direct lighting
	constexpr size_t mesh_inst_direct_light_shader_id =
		MeshInstRendering::LightPass::ShaderGrid::GetId(0, 0, 0,
		                                                static_cast<size_t>(Light::SourceType::point));
	BoundShader& mesh_inst_direct_light_shader = light_shader[mesh_inst_direct_light_shader_id];

	// get a mesh instance shader for ambient lighting
	constexpr size_t mesh_inst_ambient_light_shader_ibl_off_id =
		MeshInstRendering::LightPass::ShaderGrid::GetId(0, 0,
		                                                static_cast<size_t>(MeshInstRendering::IblPreset::off),
		                                                static_cast<size_t>(Light::SourceType::ambient));
	constexpr size_t mesh_inst_ambient_light_shader_ibl_on_id =
		MeshInstRendering::LightPass::ShaderGrid::GetId(0, 0,
		                                                static_cast<size_t>(MeshInstRendering::IblPreset::on),
		                                                static_cast<size_t>(Light::SourceType::ambient));

	// get mesh instance emissive pass shaders
	constexpr size_t mesh_inst_emissive_shader_at_off_id =
		MeshInstRendering::EmissivePass::ShaderGrid::GetId(0, static_cast<size_t>(MeshInstRendering::AlphaTest::off));
	constexpr size_t mesh_inst_emissive_shader_at_on_id =
		MeshInstRendering::EmissivePass::ShaderGrid::GetId(0, static_cast<size_t>(MeshInstRendering::AlphaTest::on));

	// creates and connects ambient pass descriptors
	auto connect_ambient_descriptors = [&]()
	{
		// ibl off
		constexpr MeshInstRendering::LightPass::AmbientLightDescriptors::IblOff ibl_off =
			MeshInstRendering::LightPass::AmbientLightDescriptors::IblOff::select;
		MeshInstRendering::LightPass::AmbientLightDescriptors ambient_ibl_off_desct_set(ibl_off);
		ambient_ibl_off_desct_set.Connect(light_shader[mesh_inst_ambient_light_shader_ibl_off_id]);
		constexpr size_t ambient_ibl_off_id = static_cast<size_t>(MeshInstRendering::IblPreset::off);
		ambient_pass_desc_set[ambient_ibl_off_id].Add(ut::Move(ambient_ibl_off_desct_set));

		// ibl on
		constexpr MeshInstRendering::LightPass::AmbientLightDescriptors::IblOn ibl_on =
			MeshInstRendering::LightPass::AmbientLightDescriptors::IblOn::select;
		MeshInstRendering::LightPass::AmbientLightDescriptors ambient_ibl_on_desct_set(ibl_on);
		ambient_ibl_on_desct_set.Connect(light_shader[mesh_inst_ambient_light_shader_ibl_on_id]);
		constexpr size_t ambient_ibl_on_id = static_cast<size_t>(MeshInstRendering::IblPreset::on);
		ambient_pass_desc_set[ambient_ibl_on_id].Add(ut::Move(ambient_ibl_on_desct_set));
	};

	// creates and connects emissive pass descriptors
	auto connect_emissive_descriptors = [&]()
	{
		// alpha test off
		constexpr MeshInstRendering::EmissivePass::Descriptors::AlphaTestOff at_off =
			MeshInstRendering::EmissivePass::Descriptors::AlphaTestOff::select;
		MeshInstRendering::EmissivePass::Descriptors emissive_at_off_desct_set(at_off);
		emissive_at_off_desct_set.Connect(emissive_shader[mesh_inst_emissive_shader_at_off_id]);
		constexpr size_t emissive_at_off_id = static_cast<size_t>(MeshInstRendering::AlphaTest::off);
		emissive_pass_desc_set[emissive_at_off_id].Add(ut::Move(emissive_at_off_desct_set));

		// alpha test on
		constexpr MeshInstRendering::EmissivePass::Descriptors::AlphaTestOn at_on =
			MeshInstRendering::EmissivePass::Descriptors::AlphaTestOn::select;
		MeshInstRendering::EmissivePass::Descriptors emissive_at_on_desct_set(at_on);
		emissive_at_on_desct_set.Connect(emissive_shader[mesh_inst_emissive_shader_at_on_id]);
		constexpr size_t emissive_at_on_id = static_cast<size_t>(MeshInstRendering::AlphaTest::on);
		emissive_pass_desc_set[emissive_at_on_id].Add(ut::Move(emissive_at_on_desct_set));
	};

	// initialize descriptors for all threads
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	ibl_pass_desc_set.Resize(thread_count);
	light_pass_desc_set.Resize(thread_count);
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		ibl_pass_desc_set[i].Connect(ibl_shader.GetFirst());
		light_pass_desc_set[i].Connect(mesh_inst_direct_light_shader);

		connect_ambient_descriptors();
		connect_emissive_descriptors();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//