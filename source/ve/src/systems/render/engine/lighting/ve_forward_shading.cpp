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
                                                         , light_shader(CreateMeshInstLightPassShader())
                                                         , ibl_shader(CreateMeshInstIblShader(ibl_mip_count))
                                                         , lightpass(CreateLightPass())
                                                         , mesh_inst_lightpass_pipeline(CreateMeshInstLightPassPipelinePermutations())
                                                         , mesh_inst_iblpass_pipeline(CreateMeshInstIblPassPipelinePermutations())
{
	ConnectDescriptors();
}

// Creates deferred shading (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param light_buffer - reference to the light buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
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
		cmd_buffer_info.usage = CmdBuffer::usage_dynamic | CmdBuffer::usage_inside_render_pass;
		cmd_buffer_info.level = CmdBuffer::level_secondary;

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

// Renders scene directly to the light buffer.
void ForwardShading::DrawTransparentGeometry(Context& context,
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
	UT_ASSERT(thread_count == lightpass_mesh_inst_desc_set.Count());

	// perform z-sorting
	SortTransparentDrawCalls(view_position, batcher.draw_calls);
	const ut::uint32 sorted_dc_count = static_cast<ut::uint32>(z_sorted_dc_indices.Count());

	// initialize secondary buffers for a parallel work
	const ut::uint32 dc_per_thread = sorted_dc_count / thread_count; // drawcalls per thread
	secondary_buffer_cache.Reset();
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		secondary_buffer_cache.Add(data.lightpass_cmd[cubeface * thread_count + i]);
		tools.device.ResetCmdBuffer(secondary_buffer_cache.GetLast().Get());
	}

	// begin a render pass
	Framebuffer& framebuffer = data.light_framebuffer[cubeface];
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

	const LightPass::MeshInstRendering::IblPreset ibl_preset = ibl_cubemap ? LightPass::MeshInstRendering::ibl_on :
	                                                                         LightPass::MeshInstRendering::ibl_off;
	const ut::uint32 last_element = offset + count - 1;
	for (ut::uint32 i = offset; i <= last_element; i++)
	{
		// skip opaque mesh instances
		const ut::uint32 sorted_id = z_sorted_dc_indices[i].first;
		MeshInstance::DrawCall& dc = batcher.draw_calls[sorted_id];
		Mesh::Subset& subset = dc.instance.mesh->subsets[dc.subset_id];
		Material& material = subset.material;
		if (material.alpha != Material::alpha_transparent)
		{
			continue;
		}

		constexpr ut::uint32 front_culling = 0;
		constexpr ut::uint32 back_culling = 1;
		constexpr ut::uint32 cull_mode_count = 2;
		ut::uint32 cull_mode = material.double_sided ? front_culling : back_culling;
		for (; cull_mode < cull_mode_count; cull_mode++)
		{
			bool first_pass = true;
			// ibl reflections pass
			if (ibl_cubemap)
			{
				const IblPass::MeshInstRendering::CullMode ibl_cull_mode = cull_mode == front_culling ?
				                                                           IblPass::MeshInstRendering::cull_front :
				                                                           IblPass::MeshInstRendering::cull_back;
				const IblPass::MeshInstRendering::AlphaMode ibl_alpha_mode = first_pass ?
				                                                             IblPass::MeshInstRendering::alpha_blend :
				                                                             IblPass::MeshInstRendering::alpha_add;
				RenderTransparentMeshInstanceIbl(context,
				                                 data,
				                                 ibl_cubemap.Get(),
				                                 view_uniform_buffer,
				                                 batcher,
				                                 ibl_cull_mode,
				                                 ibl_alpha_mode,
				                                 sorted_id,
				                                 thread_id);
				first_pass = false;
			}

			// direct light pass
			const LightPass::MeshInstRendering::CullMode lightpass_cull_mode = cull_mode == front_culling ?
			                                                                   LightPass::MeshInstRendering::cull_front :
			                                                                   LightPass::MeshInstRendering::cull_back;
			const LightPass::MeshInstRendering::AlphaMode lightpass_alpha_mode = first_pass ?
			                                                                     LightPass::MeshInstRendering::alpha_blend :
			                                                                     LightPass::MeshInstRendering::alpha_add;
			RenderTransparentMeshInstanceLights(context,
			                                    data,
			                                    lights,
			                                    view_uniform_buffer,
			                                    batcher,
			                                    ibl_preset,
			                                    lightpass_cull_mode,
			                                    lightpass_alpha_mode,
			                                    sorted_id,
			                                    thread_id);
		}
	}
}

// Applies direct lighting to the specified mesh instance.
void ForwardShading::RenderTransparentMeshInstanceLights(Context& context,
                                                         ForwardShading::ViewData& data,
                                                         Light::Sources& lights,
                                                         Buffer& view_uniform_buffer,
                                                         Batcher& batcher,
                                                         LightPass::MeshInstRendering::IblPreset ibl_preset,
                                                         LightPass::MeshInstRendering::CullMode cull_mode,
                                                         LightPass::MeshInstRendering::AlphaMode alpha_mode,
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
	const LightPass::MeshInstRendering::AlphaTest alpha_test = material.alpha == Material::alpha_masked ?
	                                                           LightPass::MeshInstRendering::alpha_test_on :
	                                                           LightPass::MeshInstRendering::alpha_test_off;

	// get stencil mode
	const LightPass::MeshInstRendering::StencilMode stencil_mode = dc.instance.highlighted ?
	                                                               LightPass::MeshInstRendering::stencil_highlighted :
	                                                               LightPass::MeshInstRendering::stencil_none;

	// accumulate light from all sources
	const size_t ambient_count = lights.ambient.Count();
	const size_t directional_count = lights.directional.Count();
	const size_t point_count = lights.point.Count();
	const size_t spot_count = lights.spot.Count();
	const size_t light_count = ambient_count + directional_count + point_count + spot_count;
	for (size_t light_id = 0; light_id < light_count; light_id++)
	{
		// figure out source type
		const Light::SourceType light_type = light_id < ambient_count ? Light::source_ambient :
		                                     light_id < (ambient_count + directional_count) ? Light::source_directional :
		                                     light_id < (ambient_count + directional_count + point_count) ?
		                                     Light::source_point : Light::source_spot;

		// extract light data
		ut::Optional<Buffer&> light_ub;
		if (light_type == Light::source_ambient)
		{
			AmbientLight& light = lights.ambient[light_id];
			AmbientLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::source_directional)
		{
			DirectionalLight& light = lights.directional[light_id - ambient_count];
			DirectionalLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::source_point)
		{
			PointLight& light = lights.point[light_id - ambient_count - directional_count];
			PointLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_ub = light_data.uniform_buffer;
		}
		else if (light_type == Light::source_spot)
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
			alpha_mode = LightPass::MeshInstRendering::alpha_add;
		}

		// calculate batch id
		const ut::uint32 batch_id = drawcall_id / batch_size;
		MeshInstance::Batch& batch = batches[batch_id];

		// bind pipeline state
		const Mesh::VertexFormat vertex_format = mesh.vertex_format;
		const Mesh::PolygonMode polygon_mode = mesh.polygon_mode;
		const size_t pipeline_state_id = LightPass::MeshInstRendering::PipelineGrid::GetId(vertex_format,
		                                                                                   ibl_preset,
		                                                                                   light_type,
		                                                                                   alpha_test,
		                                                                                   alpha_mode,
		                                                                                   cull_mode,
		                                                                                   stencil_mode,
		                                                                                   static_cast<size_t>(polygon_mode));

		// bind uniforms
		LightPass::MeshInstRendering::Descriptors& desc_set = lightpass_mesh_inst_desc_set[thread_id];
		desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
		desc_set.light_ub.BindUniformBuffer(light_ub.Get());
		desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
		desc_set.transform_ub.BindUniformBuffer(batch.transform);
		desc_set.material_ub.BindUniformBuffer(batch.material);
		desc_set.diffuse.BindImage(material.diffuse.Get());
		desc_set.normal.BindImage(material.normal.Get());
		desc_set.material.BindImage(material.material.Get());

		// bind index buffer and draw
		DrawMesh(context,
		         mesh,
		         mesh_inst_lightpass_pipeline[pipeline_state_id],
		         desc_set,
		         instance_buffer,
		         subset.index_count,
		         subset.index_offset,
		         drawcall_id % batch_size);
	}
}

// Applies direct lighting to the specified mesh instance.
void ForwardShading::RenderTransparentMeshInstanceIbl(Context& context,
                                                      ForwardShading::ViewData& data,
                                                      Image& ibl_cubemap,
                                                      Buffer& view_uniform_buffer,
                                                      Batcher& batcher,
                                                      IblPass::MeshInstRendering::CullMode cull_mode,
                                                      IblPass::MeshInstRendering::AlphaMode alpha_mode,
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
	const IblPass::MeshInstRendering::AlphaTest alpha_test = material.alpha == Material::alpha_masked ?
	                                                         IblPass::MeshInstRendering::alpha_test_on :
	                                                         IblPass::MeshInstRendering::alpha_test_off;

	// get stencil mode
	const IblPass::MeshInstRendering::StencilMode stencil_mode = dc.instance.highlighted ?
	                                                             IblPass::MeshInstRendering::stencil_highlighted :
	                                                             IblPass::MeshInstRendering::stencil_none;

	// calculate batch id
	const ut::uint32 batch_id = drawcall_id / batch_size;
	MeshInstance::Batch& batch = batches[batch_id];

	// get pipeline state
	const Mesh::VertexFormat vertex_format = mesh.vertex_format;
	const Mesh::PolygonMode polygon_mode = mesh.polygon_mode;
	const size_t pipeline_state_id = IblPass::MeshInstRendering::PipelineGrid::GetId(vertex_format,
	                                                                                 alpha_test,
	                                                                                 alpha_mode,
	                                                                                 cull_mode,
	                                                                                 stencil_mode,
	                                                                                 static_cast<size_t>(polygon_mode));

	// bind uniforms
	IblPass::MeshInstRendering::Descriptors& desc_set = iblpass_mesh_inst_desc_set[thread_id];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
	desc_set.transform_ub.BindUniformBuffer(batch.transform);
	desc_set.material_ub.BindUniformBuffer(batch.material);
	desc_set.diffuse.BindImage(material.diffuse.Get());
	desc_set.normal.BindImage(material.normal.Get());
	desc_set.material.BindImage(material.material.Get());
	desc_set.ibl_cubemap.BindImage(ibl_cubemap);

	// draw
	DrawMesh(context,
	         mesh,
	         mesh_inst_iblpass_pipeline[pipeline_state_id],
	         desc_set,
	         instance_buffer,
	         subset.index_count,
	         subset.index_offset,
	         drawcall_id % batch_size);
}

// Renders provided mesh.
void ForwardShading::DrawMesh(Context& context,
                              Mesh& mesh,
                              PipelineState& pipeline,
                              DescriptorSet& desc_set,
                              Buffer& instance_buffer,
                              ut::uint32 index_count,
                              ut::uint32 index_offset,
                              ut::uint32 instance_offset)
{
	context.BindPipelineState(pipeline);
	context.BindDescriptorSet(desc_set);
	context.BindVertexAndInstanceBuffer(mesh.vertex_buffer, 0, instance_buffer, 0);
	if (mesh.index_buffer)
	{
		context.BindIndexBuffer(mesh.index_buffer.Get(), 0, mesh.index_type);
		context.DrawIndexedInstanced(index_count,
		                             1,
		                             index_offset,
		                             0,
		                             instance_offset);
	}
	else
	{
		context.DrawInstanced(index_count, // index count here means vertex count
		                      1,
		                      index_offset, // index offset here means vertex offset
		                      instance_offset);
	}
}

// Creates a shader for the lighting pass.
ut::Array<BoundShader> ForwardShading::CreateMeshInstLightPassShader()
{
	ut::Array<BoundShader> shaders;
	constexpr size_t light_permutation_count = LightPass::MeshInstRendering::ShaderGrid::size;
	for (size_t i = 0; i < light_permutation_count; i++)
	{
		const size_t vertex_format = LightPass::MeshInstRendering::ShaderGrid::GetCoordinate<LightPass::MeshInstRendering::vertex_format_column>(i);
		const size_t ibl_preset = LightPass::MeshInstRendering::ShaderGrid::GetCoordinate<LightPass::MeshInstRendering::ibl_column>(i);
		const size_t source_type = LightPass::MeshInstRendering::ShaderGrid::GetCoordinate<LightPass::MeshInstRendering::light_type_column>(i);
		const size_t alpha_test = LightPass::MeshInstRendering::ShaderGrid::GetCoordinate<LightPass::MeshInstRendering::alpha_test_column>(i);

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
		const bool ibl_enabled = ibl_preset == LightPass::MeshInstRendering::ibl_on;
		shader_name_suffix += ibl_enabled ? "_ibl" : "_noibl";
		macro.name = "IBL";
		macro.value = ibl_enabled ? "1" : "0";
		macros.Add(ut::Move(macro));

		// light type
		const char* light_type_str;
		switch (source_type)
		{
		case Light::source_directional:
			light_type_str = "DIRECTIONAL_LIGHT";
			shader_name_suffix += "_directional";
			break;
		case Light::source_point:
			light_type_str = "POINT_LIGHT";
			shader_name_suffix += "_point";
			break;
		case Light::source_spot:
			light_type_str = "SPOT_LIGHT";
			shader_name_suffix += "_spot";
			break;
		case Light::source_ambient:
			light_type_str = "AMBIENT_LIGHT";
			shader_name_suffix += "_ambient";
			break;
		default: throw ut::Error(ut::error::not_implemented);
		}
		macro.name = light_type_str;
		macro.value = "1";
		macros.Add(ut::Move(macro));

		// alpha test
		const bool alpha_test_enabled = alpha_test == LightPass::MeshInstRendering::alpha_test_on;
		macro.name = "ALPHA_TEST";
		macro.value = alpha_test_enabled ? "1" : "0";
		macros.Add(macro);
		shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

		ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex,
		                                                            ut::String("forward_mesh_lp_vs") + shader_name_suffix,
		                                                            "VS",
		                                                            "mesh.hlsl",
		                                                            macros);

		ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
		                                                            ut::String("forward_mesh_lp_ps") + shader_name_suffix,
		                                                            "PS",
		                                                            "mesh.hlsl",
		                                                            macros);

		const bool result = shaders.Add(BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow()));
		if (!result)
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return shaders;
}

// Creates a shader for the ibl pass.
ut::Array<BoundShader> ForwardShading::CreateMeshInstIblShader(ut::uint32 ibl_mip_count)
{
	ut::Array<BoundShader> shaders;
	constexpr size_t ibl_permutation_count = IblPass::MeshInstRendering::ShaderGrid::size;
	for (size_t i = 0; i < ibl_permutation_count; i++)
	{
		const size_t vertex_format = IblPass::MeshInstRendering::ShaderGrid::GetCoordinate<IblPass::MeshInstRendering::vertex_format_column>(i);
		const size_t alpha_test = IblPass::MeshInstRendering::ShaderGrid::GetCoordinate<IblPass::MeshInstRendering::alpha_test_column>(i);

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
		const bool alpha_test_enabled = alpha_test == IblPass::MeshInstRendering::alpha_test_on;
		macro.name = "ALPHA_TEST";
		macro.value = alpha_test_enabled ? "1" : "0";
		macros.Add(macro);
		shader_name_suffix += alpha_test_enabled ? "_at_on" : "_at_off";

		ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex,
		                                                            ut::String("forward_mesh_ibl_vs") + shader_name_suffix,
		                                                            "VS",
		                                                            "mesh.hlsl",
		                                                            macros);

		ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
		                                                            ut::String("forward_mesh_ibl_ps") + shader_name_suffix,
		                                                            "PS",
		                                                            "mesh.hlsl",
		                                                            macros);

		const bool result = shaders.Add(BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow()));
		if (!result)
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return shaders;
}

// Creates a render pass for the shading techniques.
RenderPass ForwardShading::CreateLightPass()
{
	RenderTargetSlot depth_slot(tools.formats.depth_stencil, RenderTargetSlot::load_extract, RenderTargetSlot::store_save, false);
	RenderTargetSlot color_slot(tools.formats.light_buffer, RenderTargetSlot::load_extract, RenderTargetSlot::store_save, false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot).MoveOrThrow();
}

// Creates a pipeline state to apply lighting.
ut::Result<PipelineState, ut::Error> ForwardShading::CreateMeshInstLightPassPipeline(Mesh::VertexFormat vertex_format,
                                                                                     Mesh::PolygonMode polygon_mode,
                                                                                     LightPass::MeshInstRendering::AlphaTest alpha_test,
                                                                                     LightPass::MeshInstRendering::AlphaMode alpha_mode,
                                                                                     LightPass::MeshInstRendering::IblPreset ibl_preset,
                                                                                     LightPass::MeshInstRendering::CullMode cull_mode,
                                                                                     LightPass::MeshInstRendering::StencilMode stencil_mode,
                                                                                     Light::SourceType source_type)
{
	PipelineState::Info info;
	const size_t shader_id = LightPass::MeshInstRendering::ShaderGrid::GetId(vertex_format,
	                                                                         ibl_preset,
	                                                                         source_type,
	                                                                         alpha_test);
	UT_ASSERT(light_shader[shader_id].stages[Shader::vertex]);
	UT_ASSERT(light_shader[shader_id].stages[Shader::pixel]);

	const ut::uint32 stencil_mask = stencil_mode == LightPass::MeshInstRendering::stencil_highlighted ?
	                                                stencilref_highlight : 0x0;

	info.stages[Shader::vertex] = light_shader[shader_id].stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = light_shader[shader_id].stages[Shader::pixel].Get();
	info.input_assembly_state = Mesh::CreateIaState(vertex_format, polygon_mode, Mesh::Instancing::on);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::replace;
	info.depth_stencil_state.back.compare_mask = 0xffffffff;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xffffffff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = Mesh::GetRasterizerPolygonMode(polygon_mode);
	info.rasterization_state.cull_mode = cull_mode == LightPass::MeshInstRendering::cull_back ?
	                                                  RasterizationState::back_culling :
	                                                  RasterizationState::no_culling;
	if (alpha_mode == LightPass::MeshInstRendering::alpha_blend)
	{
		Blending blending(true,
		                  Blending::src_alpha,
		                  Blending::inverted_src_alpha,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == LightPass::MeshInstRendering::alpha_add)
	{
		Blending blending(true,
		                  Blending::src_alpha,
		                  Blending::one,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	
	return tools.device.CreatePipelineState(ut::Move(info), lightpass);
}

// Creates a pipeline state to apply ibl reflections.
ut::Result<PipelineState, ut::Error> ForwardShading::CreateMeshInstIblPassPipeline(Mesh::VertexFormat vertex_format,
                                                                                   Mesh::PolygonMode polygon_mode,
                                                                                   IblPass::MeshInstRendering::AlphaTest alpha_test,
                                                                                   IblPass::MeshInstRendering::AlphaMode alpha_mode,
                                                                                   IblPass::MeshInstRendering::CullMode cull_mode,
                                                                                   IblPass::MeshInstRendering::StencilMode stencil_mode)
{
	PipelineState::Info info;
	const size_t shader_id = IblPass::MeshInstRendering::ShaderGrid::GetId(vertex_format, alpha_test);
	UT_ASSERT(ibl_shader[shader_id].stages[Shader::vertex]);
	UT_ASSERT(ibl_shader[shader_id].stages[Shader::pixel]);

	const ut::uint32 stencil_mask = stencil_mode == LightPass::MeshInstRendering::stencil_highlighted ?
	                                                stencilref_highlight : 0x0;

	info.stages[Shader::vertex] = ibl_shader[shader_id].stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = ibl_shader[shader_id].stages[Shader::pixel].Get();
	info.input_assembly_state = Mesh::CreateIaState(vertex_format, polygon_mode, Mesh::Instancing::on);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::less_or_equal;
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::less_or_equal;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::replace;
	info.depth_stencil_state.back.compare_mask = 0xffffffff;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xffffffff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = Mesh::GetRasterizerPolygonMode(polygon_mode);
	info.rasterization_state.cull_mode = cull_mode == IblPass::MeshInstRendering::cull_back ?
	                                                  RasterizationState::back_culling :
	                                                  RasterizationState::no_culling;
	if (alpha_mode == IblPass::MeshInstRendering::alpha_blend)
	{
		Blending blending(true,
		                  Blending::one,
		                  Blending::inverted_src_alpha,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	else if (alpha_mode == IblPass::MeshInstRendering::alpha_add)
	{
		Blending blending(true,
		                  Blending::one,
		                  Blending::one,
		                  Blending::add,
		                  Blending::one,
		                  Blending::one,
		                  Blending::max,
		                  0xf);
		info.blend_state.attachments.Add(blending);
	}
	
	return tools.device.CreatePipelineState(ut::Move(info), lightpass);
}

// Creates all possible lighting pipeline state permutations for a mesh instance.
ut::Array<PipelineState> ForwardShading::CreateMeshInstLightPassPipelinePermutations()
{
	// calculate multithreading data to create light pass pipeline states
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t light_permutation_count = LightPass::MeshInstRendering::PipelineGrid::size;
	const size_t light_permutations_per_thread = light_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<PipelineState> > lightpass_pipeline_mt_cache(light_permutation_count);

	// multithreading job to create lightpass pipeline
	auto mesh_inst_lp_pipeline_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::vertex_format_column>(i);
			const size_t alpha_test = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::alpha_test_column>(i);
			const size_t alpha_mode = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::alpha_mode_column>(i);
			const size_t ibl_preset = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::ibl_column>(i);
			const size_t source_type = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::light_type_column>(i);
			const size_t cull_mode = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::cull_mode_column>(i);
			const size_t stencil_mode = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::stencil_mode_column>(i);
			const size_t polygon_mode = LightPass::MeshInstRendering::PipelineGrid::GetCoordinate<LightPass::MeshInstRendering::polygon_mode_column>(i);

			ut::Result<PipelineState, ut::Error> pipeline = CreateMeshInstLightPassPipeline(static_cast<Mesh::VertexFormat>(vertex_format),
			                                                                                static_cast<Mesh::PolygonMode>(polygon_mode),
			                                                                                static_cast<LightPass::MeshInstRendering::AlphaTest>(alpha_test),
			                                                                                static_cast<LightPass::MeshInstRendering::AlphaMode>(alpha_mode),
			                                                                                static_cast<LightPass::MeshInstRendering::IblPreset>(ibl_preset),
			                                                                                static_cast<LightPass::MeshInstRendering::CullMode>(cull_mode),
			                                                                                static_cast<LightPass::MeshInstRendering::StencilMode>(stencil_mode),
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

// Creates all possible ibl pipeline state permutations for a mesh instance.
ut::Array<PipelineState> ForwardShading::CreateMeshInstIblPassPipelinePermutations()
{
	// calculate multithreading data to create ibl pass pipeline states
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t ibl_permutation_count = IblPass::MeshInstRendering::PipelineGrid::size;
	const size_t ibl_permutations_per_thread = ibl_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<PipelineState> > ibl_pipeline_mt_cache(ibl_permutation_count);

	// multithreading job to create iblpass pipeline
	auto mesh_inst_ibl_pipeline_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			const size_t vertex_format = IblPass::MeshInstRendering::PipelineGrid::GetCoordinate<IblPass::MeshInstRendering::vertex_format_column>(i);
			const size_t alpha_test = IblPass::MeshInstRendering::PipelineGrid::GetCoordinate<IblPass::MeshInstRendering::alpha_test_column>(i);
			const size_t alpha_mode = IblPass::MeshInstRendering::PipelineGrid::GetCoordinate<IblPass::MeshInstRendering::alpha_mode_column>(i);
			const size_t cull_mode = IblPass::MeshInstRendering::PipelineGrid::GetCoordinate<IblPass::MeshInstRendering::cull_mode_column>(i);
			const size_t stencil_mode = IblPass::MeshInstRendering::PipelineGrid::GetCoordinate<IblPass::MeshInstRendering::stencil_mode_column>(i);
			const size_t polygon_mode = IblPass::MeshInstRendering::PipelineGrid::GetCoordinate<IblPass::MeshInstRendering::polygon_mode_column>(i);

			ut::Result<PipelineState, ut::Error> pipeline = CreateMeshInstIblPassPipeline(static_cast<Mesh::VertexFormat>(vertex_format),
			                                                                              static_cast<Mesh::PolygonMode>(polygon_mode),
			                                                                              static_cast<IblPass::MeshInstRendering::AlphaTest>(alpha_test),
			                                                                              static_cast<IblPass::MeshInstRendering::AlphaMode>(alpha_mode),
			                                                                              static_cast<IblPass::MeshInstRendering::CullMode>(cull_mode),
			                                                                              static_cast<IblPass::MeshInstRendering::StencilMode>(stencil_mode));
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

// Connects all descriptor sets to the corresponding shaders.
void ForwardShading::ConnectDescriptors()
{
	UT_ASSERT(light_shader.Count() != 0);
	UT_ASSERT(ibl_shader.Count() != 0);
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	lightpass_mesh_inst_desc_set.Resize(thread_count);
	iblpass_mesh_inst_desc_set.Resize(thread_count);
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		lightpass_mesh_inst_desc_set[i].Connect(light_shader.GetFirst());
		iblpass_mesh_inst_desc_set[i].Connect(ibl_shader.GetFirst());
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
		// skip opaque draw calls
		const MeshInstance::DrawCall& dc = draw_list[dc_index];
		const Mesh::Subset& subset = dc.instance.mesh->subsets[dc.subset_id];
		const Material& material = subset.material;
		if (material.alpha != Material::alpha_transparent)
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

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//