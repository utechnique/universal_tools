//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_view.h"
#include "systems/render/engine/lighting/ve_deferred_shading.h"
#include "systems/render/engine/policy/ve_render_mesh_instance_policy.h"
#include "systems/render/engine/ve_render_stencil_ref.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
DeferredShading::DeferredShading(Toolset& toolset,
                                 ut::uint32 ibl_mip_count) : tools(toolset)
                                                           , fullscreen_quad(tools.rc_mgr.fullscreen_quad->subsets.GetFirst())
                                                           , mesh_inst_gpass_shader(CreateMeshInstGPassShaders())
                                                           , light_shader(CreateLightPassShaders())
                                                           , emissive_shader(CreateEmissiveShader())
                                                           , ibl_shader(CreateIblShader(ibl_mip_count))
                                                           , geometry_pass(CreateMeshInstGeometryPass())
                                                           , light_pass(CreateLightPass())
                                                           , mesh_inst_gpass_pipeline(CreateMeshInstGPassPipelinePermutations())
                                                           , light_pipeline(CreateLightPassPipelinePermutations())
                                                           , emissive_pipeline(CreateEmissivePipeline())
                                                           , ibl_pipeline(CreateIblPipeline())
                                                           , ambient_pass_desc_set { AmbientPassDescriptors(AmbientPassDescriptors::IblOff::select),
																					 AmbientPassDescriptors(AmbientPassDescriptors::IblOn::select) }
{
	ConnectDescriptors();
}

// Creates deferred shading (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param light_buffer - reference to the light buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @param is_cube - 'true' to create as a cubemap.
//    @return - a new DeferredShading::ViewData object or error if failed.
ut::Result<DeferredShading::ViewData, ut::Error> DeferredShading::CreateViewData(Target& depth_stencil,
                                                                                 Target& light_buffer,
                                                                                 ut::uint32 width,
                                                                                 ut::uint32 height,
                                                                                 bool is_cube)
{
	const ut::uint32 face_count = is_cube ? 6 : 1;

	// target info
	Target::Info info;
	info.type = is_cube ? Image::Type::cubic : Image::Type::planar;
	info.usage = Target::Info::Usage::color;
	info.mip_count = 1;
	info.width = width;
	info.height = height;
	info.depth = 1;

	// base color
	info.format = tools.formats.gbuffer_base_color;
	ut::Result<Target, ut::Error> base_color = tools.device.CreateTarget(info);
	if (!base_color)
	{
		return ut::MakeError(base_color.MoveAlt());
	}

	// normal
	info.format = tools.formats.gbuffer_normal;
	ut::Result<Target, ut::Error> normal = tools.device.CreateTarget(info);
	if (!normal)
	{
		return ut::MakeError(normal.MoveAlt());
	}

	// emissive
	info.format = tools.formats.gbuffer_emissive;
	ut::Result<Target, ut::Error> emissive = tools.device.CreateTarget(info);
	if (!emissive)
	{
		return ut::MakeError(emissive.MoveAlt());
	}

	// depth
	info.format = tools.formats.depth_stencil;
	info.usage = Target::Info::Usage::depth;
	ut::Result<Target, ut::Error> depth = tools.device.CreateTarget(info);
	if (!depth)
	{
		return ut::MakeError(depth.MoveAlt());
	}

	// geometry pass framebuffer
	ut::Array<Framebuffer> gpass_framebuffer;
	for (ut::uint32 face_id = 0; face_id < face_count; face_id++)
	{
		ut::Array<Framebuffer::Attachment> color_targets;
		color_targets.Add(Framebuffer::Attachment(base_color.Get(), face_id));
		color_targets.Add(Framebuffer::Attachment(normal.Get(), face_id));
		color_targets.Add(Framebuffer::Attachment(emissive.Get(), face_id));
		ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(geometry_pass,
		                                                                                ut::Move(color_targets),
		                                                                                Framebuffer::Attachment(depth_stencil, face_id));
		if (!framebuffer)
		{
			return ut::MakeError(framebuffer.MoveAlt());
		}
		gpass_framebuffer.Add(framebuffer.Move());
	}

	// light pass framebuffer
	ut::Array<Framebuffer> light_framebuffer;
	for (ut::uint32 face_id = 0; face_id < face_count; face_id++)
	{
		ut::Array<Framebuffer::Attachment> color_targets;
		color_targets.Add(Framebuffer::Attachment(light_buffer, face_id));
		ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(light_pass,
		                                                                                ut::Move(color_targets),
		                                                                                Framebuffer::Attachment(depth_stencil, face_id));
		if (!framebuffer)
		{
			return ut::MakeError(framebuffer.MoveAlt());
		}
		light_framebuffer.Add(framebuffer.Move());
	}

	// secondary command buffers
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	const ut::uint32 secondary_buffer_count = thread_count * (is_cube ? 6 : 1);
	ut::Array<CmdBuffer> gpass_cmd;
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

		gpass_cmd.Add(cmd_buffer.Move());
	}

	DeferredShading::ViewData data =
	{
		base_color.Move(),
		normal.Move(),
		emissive.Move(),
		depth.Move(),
		ut::Move(gpass_framebuffer),
		ut::Move(light_framebuffer),
		ut::Move(gpass_cmd),
	};

	return ut::Move(data);
}

// Renders scnene to the g-buffer.
void DeferredShading::BakeOpaqueGeometry(Context& context,
                                         Target& depth_stencil,
                                         DeferredShading::ViewData& data,
                                         Buffer& view_uniform_buffer,
                                         Batcher& batcher,
                                         Image::Cube::Face cubeface)
{
	BakeOpaqueMeshInstances(context, data, view_uniform_buffer, batcher, cubeface);

	// copy depth to the intermediate buffer
	context.SetTargetState(depth_stencil, Target::Info::State::transfer_src);
	context.SetTargetState(data.depth, Target::Info::State::transfer_dst);
	context.CopyTarget(data.depth, depth_stencil, static_cast<ut::uint32>(cubeface), 1);
}

// Applies lighting techniques to the provided target.
void DeferredShading::Shade(Context& context,
                            DeferredShading::ViewData& data,
                            Buffer& view_uniform_buffer,
                            Light::Sources& lights,
                            ut::Optional<Image&> ibl_cubemap,
                            Image::Cube::Face cubeface)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();

	// check if ibl is enabled
	const LightPass::IblPreset ibl_preset = ibl_cubemap ?
	                                        LightPass::IblPreset::on :
	                                        LightPass::IblPreset::off;

	// set the g-buffer as a shader resource
	ut::Ref<Target> transition_targets[] = { data.base_color,
	                                         data.normal,
	                                         data.emissive,
	                                         data.depth };
	context.SetTargetState<4>(transition_targets, Target::Info::State::resource);

	// begin the light pass
	Framebuffer& light_framebuffer = data.light_framebuffer[static_cast<ut::uint32>(cubeface)];
	const Framebuffer::Info& fb_info = light_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(light_pass, light_framebuffer, render_area, ut::Color<4>(0));

	// set quad vertex buffer
	context.BindVertexBuffer(fullscreen_quad.vertex_buffer.GetRef(), 0);

	// image based lighting
	ShadeIblReflection(context,
	                   data,
	                   view_uniform_buffer,
	                   ibl_cubemap,
	                   cubeface);

	// indirect (ambient lights)
	ShadeAmbientLight(context,
	                  data,
	                  view_uniform_buffer,
	                  lights,
	                  ibl_preset,
	                  cubeface);

	// direct (directional, point and spot lights)
	ShadeDirectLight(context,
	                 data,
	                 view_uniform_buffer,
	                 lights,
	                 ibl_preset,
	                 cubeface);

	// emissive
	ShadeEmissive(context, data, cubeface);

	// finish the light pass
	context.EndRenderPass();
}

// Renders mesh instance units to the g-buffer.
void DeferredShading::BakeOpaqueMeshInstances(Context& context,
                                              DeferredShading::ViewData& data,
                                              Buffer& view_uniform_buffer,
                                              Batcher& batcher,
                                              Image::Cube::Face cubeface)
{
	// get the number of available threads
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	UT_ASSERT(thread_count == gpass_mesh_inst_desc_set.Count());

	// get the number of drawcalls
	ut::Array<MeshInstance::DrawCall>& draw_list = batcher.draw_calls;
	const ut::uint32 dc_count = static_cast<ut::uint32>(draw_list.Count());

	// initialize secondary buffers for a parallel work
	secondary_buffer_cache.Reset();
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		secondary_buffer_cache.Add(data.gpass_cmd[static_cast<ut::uint32>(cubeface) * thread_count + i]);
		tools.device.ResetCmdBuffer(secondary_buffer_cache.GetLast().Get());
	}

	// begin a render pass
	Framebuffer& geometry_framebuffer = data.geometry_framebuffer[static_cast<ut::uint32>(cubeface)];
	const Framebuffer::Info& fb_info = geometry_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(geometry_pass, geometry_framebuffer, render_area, ut::Color<4>(0), 1.0f, 0, true);

	// parallelize drawcalls
	const ut::uint32 dc_per_thread = dc_count / thread_count; // drawcalls per thread
	ut::uint32 offset = 0; // offset from the first drawcall in the batcher
	for (ut::uint32 thread_id = 0; thread_id < thread_count; thread_id++)
	{
		ut::uint32 count = dc_per_thread + (thread_id == 0 ? (dc_count % thread_count) : 0);

		auto record_commands = [&, thread_id, offset, count](Context& deferred_context)
		{
			BakeOpaqueMeshInstancesJob(deferred_context,
			                           data,
			                           view_uniform_buffer,
			                           batcher,
			                           thread_id,
			                           offset,
			                           count);
		};

		auto job = [&, thread_id, record_commands]()
		{
			tools.device.Record(secondary_buffer_cache[thread_id],
			                    record_commands,
			                    geometry_pass,
			                    geometry_framebuffer);
		};

		tools.scheduler.Enqueue(ut::MakeUnique< ut::Task<void()> >(job));

		offset += count;
	}
	tools.scheduler.WaitForCompletion();

	// finish render pass
	context.ExecuteSecondaryBuffers(secondary_buffer_cache);
	context.EndRenderPass();
}

// Helper function to draw a mesh subset.
inline void PerformMeshInstDrawCall(Context& context,
                                    Buffer* index_buffer,
                                    ut::uint32 index_offset,
                                    ut::uint32 index_count,
                                    ut::uint32 instance_count,
                                    ut::uint32 call_id,
                                    ut::uint32 batch_size)
{
	if (instance_count == 0)
	{
		return;
	}

	const ut::uint32 first_instance_id = (call_id + 1 - instance_count) % batch_size;
	if (index_buffer == nullptr)
	{
		context.DrawInstanced(index_count,
		                      instance_count,
		                      index_offset,
		                      first_instance_id);
	}
	else
	{
		context.DrawIndexedInstanced(index_count,
		                             instance_count,
		                             index_offset,
		                             0,
		                             first_instance_id);
	}
}

// Renders specified range of mesh instances.
void DeferredShading::BakeOpaqueMeshInstancesJob(Context& context,
                                                 DeferredShading::ViewData& data,
                                                 Buffer& view_uniform_buffer,
                                                 Batcher& batcher,
                                                 ut::uint32 thread_id,
                                                 ut::uint32 offset,
                                                 ut::uint32 count)
{
	if (count == 0)
	{
		return;
	}

	// get batcher data
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	ut::Array<MeshInstance::DrawCall>& draw_list = batcher.draw_calls;
	Buffer& instance_buffer = batcher.instance_buffer;
	ut::Array<MeshInstance::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	const ut::uint32 batch_size = batcher.GetBatchSize();

	// set common uniforms
	GeometryPass::MeshInstRendering::Descriptors& desc_set = gpass_mesh_inst_desc_set[thread_id];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);

	// variables tracking if something changes between iterations
	Mesh::VertexFormat prev_vertex_format = Mesh::VertexFormat::count;
	Mesh::PolygonMode prev_polygon_mode = Mesh::PolygonMode::count;
	GeometryPass::MeshInstRendering::AlphaMode prev_alpha_mode = GeometryPass::MeshInstRendering::AlphaMode::count;
	GeometryPass::MeshInstRendering::CullMode prev_cull_mode = GeometryPass::MeshInstRendering::CullMode::count;
	GeometryPass::MeshInstRendering::StencilMode prev_stencil_mode = GeometryPass::MeshInstRendering::StencilMode::count;
	Map* prev_base_color_ptr = nullptr;
	Map* prev_normal_ptr = nullptr;
	Map* prev_metallic_roughness_ptr = nullptr;
	Map* prev_occlusion_ptr = nullptr;
	Map* prev_emissive_ptr = nullptr;
	Buffer* prev_vertex_buffer = nullptr;
	Buffer* prev_index_buffer = nullptr;
	ut::uint32 prev_index_offset = 0;
	ut::uint32 prev_index_count = 0;
	ut::uint32 prev_batch_id = static_cast<ut::uint32>(batches.Count());

	// number of elements to draw at once
	ut::uint32 instance_count = 0;

	// iterate primitive groups and try to merge them in the
	// least possible amount of draw calls
	const ut::uint32 last_element = offset + count - 1;
	for (ut::uint32 i = offset; i <= last_element; i++)
	{
		// extract material
		MeshInstance::DrawCall& dc = draw_list[i];
		Mesh& mesh = dc.instance.mesh.Get();
		Mesh::Subset& subset = mesh.subsets[dc.subset_id];
		Material& material = subset.material;
		const bool is_transparent = material.alpha == Material::Alpha::transparent;

		// skip transparent and unlit materials
		const bool skip = is_transparent ||
		                  material.unlit ||
		                  dc.instance.force_forward_renderer;
		if (skip)
		{
			PerformMeshInstDrawCall(context, prev_index_buffer,
			                        prev_index_offset, prev_index_count,
			                        instance_count, i - 1, batch_size);
			instance_count = 0;
			continue;
		}

		// calculate batch id
		const ut::uint32 batch_id = i / batch_size;
		MeshInstance::Batch& batch = batches[batch_id];

		// material maps
		Map* base_color_ptr = &material.base_color.Get();
		Map* normal_ptr = &material.normal.Get();
		Map* metallic_roughness_ptr = &material.metallic_roughness.Get();
		Map* occlusion_ptr = &material.occlusion.Get();
		Map* emissive_ptr = &material.emissive.Get();

		// buffers
		Mesh::VertexBuffer* vertex_buffer = subset.vertex_buffer.Get();
		UT_ASSERT(vertex_buffer);
		Mesh::IndexBuffer* index_buffer = subset.index_buffer.Get();

		// index count
		const ut::uint32 index_offset = subset.offset;
		const ut::uint32 index_count = subset.count;

		// check pipeline state
		const Mesh::VertexFormat vertex_format = vertex_buffer->format;
		const Mesh::PolygonMode polygon_mode = subset.polygon_mode;
		const GeometryPass::MeshInstRendering::AlphaMode alpha_mode = material.alpha == Material::Alpha::masked ?
		                                                              GeometryPass::MeshInstRendering::AlphaMode::alpha_test :
		                                                              GeometryPass::MeshInstRendering::AlphaMode::opaque;
		const GeometryPass::MeshInstRendering::CullMode cull_mode = material.double_sided ?
		                                                            GeometryPass::MeshInstRendering::CullMode::none :
		                                                            GeometryPass::MeshInstRendering::CullMode::back;
		const GeometryPass::MeshInstRendering::StencilMode stencil_mode = dc.instance.highlighted ?
		                                                                  GeometryPass::MeshInstRendering::StencilMode::opaque_and_highlighted :
		                                                                  GeometryPass::MeshInstRendering::StencilMode::opaque;
		bool pipeline_changed = prev_vertex_format != vertex_format ||
		                        prev_polygon_mode != polygon_mode ||
		                        prev_cull_mode != cull_mode ||
		                        prev_alpha_mode != alpha_mode ||
		                        prev_stencil_mode != stencil_mode;

		// check if at least one shader resource has changed
		const bool shader_rc_changed = batch_id != prev_batch_id ||
		                               base_color_ptr != prev_base_color_ptr ||
		                               normal_ptr != prev_normal_ptr ||
		                               metallic_roughness_ptr != prev_metallic_roughness_ptr ||
		                               occlusion_ptr != prev_occlusion_ptr ||
		                               emissive_ptr != prev_emissive_ptr;

		// check buffers
		const bool vertex_buffer_changed = prev_vertex_buffer != vertex_buffer;
		const bool index_buffer_changed = prev_index_buffer != index_buffer;
		
		// check index count
		const bool indices_changed = prev_index_offset != index_offset ||
		                             prev_index_count != index_count;

		// check if at least one factor has changed since the previous iteration
		const bool state_changed = pipeline_changed ||
		                           shader_rc_changed ||
		                           vertex_buffer_changed ||
		                           index_buffer_changed ||
		                           indices_changed;

		// draw previous instance group
		if (state_changed && (i != offset))
		{
			PerformMeshInstDrawCall(context, prev_index_buffer,
			                        prev_index_offset, prev_index_count,
			                        instance_count, i - 1, batch_size);
			instance_count = 0;
		}
		instance_count++;

		// set pipeline state
		if (pipeline_changed)
		{
			const size_t pipeline_state_id = GeometryPass::MeshInstRendering::PipelineGrid::GetId(static_cast<size_t>(vertex_format),
			                                                                                      static_cast<size_t>(alpha_mode),
			                                                                                      static_cast<size_t>(cull_mode),
			                                                                                      static_cast<size_t>(stencil_mode),
			                                                                                      static_cast<size_t>(polygon_mode));
			context.BindPipelineState(mesh_inst_gpass_pipeline[pipeline_state_id]);

			prev_vertex_format = vertex_format;
			prev_polygon_mode = polygon_mode;
			prev_alpha_mode = alpha_mode;
			prev_cull_mode = cull_mode;
			prev_stencil_mode = stencil_mode;
		}

		// bind descriptors
		if (shader_rc_changed)
		{
			desc_set.transform_ub.BindUniformBuffer(batch.transform);
			desc_set.material_ub.BindUniformBuffer(batch.material);
			desc_set.base_color.BindImage(material.base_color.Get());
			desc_set.normal.BindImage(material.normal.Get());
			desc_set.metallic_roughness.BindImage(material.metallic_roughness.Get());
			desc_set.occlusion.BindImage(material.occlusion.Get());
			desc_set.emissive.BindImage(material.emissive.Get());
			context.BindDescriptorSet(desc_set);

			prev_batch_id = batch_id;
			prev_base_color_ptr = base_color_ptr;
			prev_normal_ptr = normal_ptr;
			prev_metallic_roughness_ptr = metallic_roughness_ptr;
			prev_occlusion_ptr = occlusion_ptr;
			prev_emissive_ptr = emissive_ptr;
		}

		// bind vertex buffer
		if (vertex_buffer_changed)
		{
			context.BindVertexAndInstanceBuffer(*vertex_buffer, 0, instance_buffer, 0);
			prev_vertex_buffer = vertex_buffer;
		}
		
		// bind index buffer
		if (index_buffer_changed)
		{
			if (index_buffer != nullptr)
			{
				context.BindIndexBuffer(*index_buffer, 0, index_buffer->format);
			}
			prev_index_buffer = index_buffer;
		}

		// update indices count and offset
		if (indices_changed)
		{
			prev_index_offset = index_offset;
			prev_index_count = index_count;
		}

		// draw last element
		if (i == last_element)
		{
			PerformMeshInstDrawCall(context, index_buffer,
			                        index_offset, index_count,
			                        instance_count, i, batch_size);
		}
	}
}

// Applies IBL lighting.
void DeferredShading::ShadeIblReflection(Context& context,
                                         DeferredShading::ViewData& data,
                                         Buffer& view_uniform_buffer,
                                         ut::Optional<Image&> ibl_cubemap,
                                         Image::Cube::Face cubeface)
{
	if (!ibl_cubemap)
	{
		return;
	}

	if (data.depth.GetInfo().type == Image::Type::cubic)
	{
		ibl_desc_set.depth.BindCubeFace(data.depth.GetImage(), cubeface);
		ibl_desc_set.base_color.BindCubeFace(data.base_color.GetImage(), cubeface);
		ibl_desc_set.normal.BindCubeFace(data.normal.GetImage(), cubeface);
	}
	else
	{
		ibl_desc_set.depth.BindImage(data.depth.GetImage());
		ibl_desc_set.base_color.BindImage(data.base_color.GetImage());
		ibl_desc_set.normal.BindImage(data.normal.GetImage());
	}

	ibl_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	ibl_desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);
	ibl_desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
	ibl_desc_set.ibl_cubemap.BindImage(ibl_cubemap.Get());
	context.BindPipelineState(ibl_pipeline);
	context.BindDescriptorSet(ibl_desc_set);
	context.Draw(6, 0);
}

// Applies ambient lighting.
void DeferredShading::ShadeAmbientLight(Context& context,
                                        DeferredShading::ViewData& data,
                                        Buffer& view_uniform_buffer,
                                        Light::Sources& lights,
                                        LightPass::IblPreset ibl_preset,
                                        Image::Cube::Face cubeface)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();

	const size_t ambient_light_count = lights.ambient.Count();
	if (ambient_light_count == 0)
	{
		return;
	}

	// set shader resources
	AmbientPassDescriptors& desc_set = ambient_pass_desc_set[static_cast<size_t>(ibl_preset)];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);

	// bind g-buffer
	if (data.depth.GetInfo().type == Image::Type::cubic)
	{
		desc_set.depth.BindCubeFace(data.depth.GetImage(), cubeface);
		desc_set.base_color.BindCubeFace(data.base_color.GetImage(), cubeface);
		desc_set.emissive.BindCubeFace(data.emissive.GetImage(), cubeface);

		if (ibl_preset == LightPass::IblPreset::on)
		{
			desc_set.normal.BindCubeFace(data.normal.GetImage(), cubeface);
		}
	}
	else
	{
		desc_set.depth.BindImage(data.depth.GetImage());
		desc_set.base_color.BindImage(data.base_color.GetImage());
		desc_set.emissive.BindImage(data.emissive.GetImage());

		if (ibl_preset == LightPass::IblPreset::on)
		{
			desc_set.normal.BindImage(data.normal.GetImage());
		}
	}

	const size_t pipeline_id = LightPass::Grid::GetId(static_cast<size_t>(ibl_preset),
	                                                  static_cast<size_t>(Light::SourceType::ambient));
	context.BindPipelineState(light_pipeline[pipeline_id]);
	for (size_t i = 0; i < ambient_light_count; i++)
	{
		AmbientLight& light = lights.ambient[i];
		AmbientLight::FrameData& light_data = light.data->frames[current_frame_id];
		desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
		context.BindDescriptorSet(desc_set);
		context.Draw(6, 0);
	}
}

// Applies directional + spot + point lighting.
void DeferredShading::ShadeDirectLight(Context& context,
                                       DeferredShading::ViewData& data,
                                       Buffer& view_uniform_buffer,
                                       Light::Sources& lights,
                                       LightPass::IblPreset ibl_preset,
                                       Image::Cube::Face cubeface)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();

	// set shader resources
	light_pass_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	light_pass_desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);

	// bind g-buffer targets as resources
	if (data.depth.GetInfo().type == Image::Type::cubic)
	{
		light_pass_desc_set.depth.BindCubeFace(data.depth.GetImage(), cubeface);
		light_pass_desc_set.base_color.BindCubeFace(data.base_color.GetImage(), cubeface);
		light_pass_desc_set.normal.BindCubeFace(data.normal.GetImage(), cubeface);
	}
	else
	{
		light_pass_desc_set.depth.BindImage(data.depth.GetImage());
		light_pass_desc_set.base_color.BindImage(data.base_color.GetImage());
		light_pass_desc_set.normal.BindImage(data.normal.GetImage());
	}

	// directional lights
	const size_t directional_light_count = lights.directional.Count();
	if (directional_light_count > 0)
	{
		const size_t pipeline_id = LightPass::Grid::GetId(static_cast<size_t>(ibl_preset),
			static_cast<size_t>(Light::SourceType::directional));
		context.BindPipelineState(light_pipeline[pipeline_id]);
		for (size_t i = 0; i < directional_light_count; i++)
		{
			DirectionalLight& light = lights.directional[i];
			DirectionalLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_pass_desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
			context.BindDescriptorSet(light_pass_desc_set);
			context.Draw(6, 0);
		}
	}

	// point lights
	const size_t point_light_count = lights.point.Count();
	if (point_light_count > 0)
	{
		const size_t pipeline_id = LightPass::Grid::GetId(static_cast<size_t>(ibl_preset),
			static_cast<size_t>(Light::SourceType::point));
		context.BindPipelineState(light_pipeline[pipeline_id]);
		for (size_t i = 0; i < point_light_count; i++)
		{
			PointLight& light = lights.point[i];
			PointLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_pass_desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
			context.BindDescriptorSet(light_pass_desc_set);
			context.Draw(6, 0);
		}
	}

	// spot lights
	const size_t spot_light_count = lights.spot.Count();
	if (spot_light_count > 0)
	{
		const size_t pipeline_id = LightPass::Grid::GetId(static_cast<size_t>(ibl_preset),
			static_cast<size_t>(Light::SourceType::spot));
		context.BindPipelineState(light_pipeline[pipeline_id]);
		for (size_t i = 0; i < spot_light_count; i++)
		{
			SpotLight& light = lights.spot[i];
			SpotLight::FrameData& light_data = light.data->frames[current_frame_id];
			light_pass_desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
			context.BindDescriptorSet(light_pass_desc_set);
			context.Draw(6, 0);
		}
	}
}

// Applies emissive lighting.
void DeferredShading::ShadeEmissive(Context& context,
                                    DeferredShading::ViewData& data,
                                    Image::Cube::Face cubeface)
{
	if (data.depth.GetInfo().type == Image::Type::cubic)
	{
		emissive_pass_desc_set.emissive.BindCubeFace(data.emissive.GetImage(), cubeface);
	}
	else
	{
		emissive_pass_desc_set.emissive.BindImage(data.emissive.GetImage());
	}

	emissive_pass_desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);

	context.BindPipelineState(emissive_pipeline);
	context.BindDescriptorSet(emissive_pass_desc_set);
	context.Draw(6, 0);
}

// Creates shaders for rendering geometry to the g-buffer.
ut::Array<BoundShader> DeferredShading::CreateMeshInstGPassShaders()
{
	// calculate multithreading data
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	constexpr size_t shader_permutation_count = GeometryPass::MeshInstRendering::ShaderGrid::size;
	const size_t shader_permutations_per_thread = shader_permutation_count / thread_count;
	ut::Array< ut::UniquePtr<BoundShader> > temp_shader_cache(shader_permutation_count);

	// multithreading job
	auto load_shader_job = [&](ut::uint32 offset, ut::uint32 count)
	{
		for (ut::uint32 i = offset; i < offset + count; i++)
		{
			ut::String shader_name_suffix;
			Shader::Macros macros;
			Shader::MacroDefinition macro;

			// enable instancing
			macro.name = "INSTANCING";
			macro.value = "1";
			macros.Add(macro);

			// deferred pass
			macro.name = "DEFERRED_PASS";
			macro.value = "1";
			macros.Add(macro);

			// batch size
			macro.name = "BATCH_SIZE";
			macro.value = ut::Print(Batcher::CalculateBatchSize(tools.device));
			macros.Add(macro);

			// vertex traits
			Mesh::VertexFormat vertex_format =
				static_cast<Mesh::VertexFormat>(
					GeometryPass::MeshInstRendering::ShaderGrid::GetCoordinate<GeometryPass::MeshInstRendering::vertex_format_column>(i));
			macros += Mesh::GenerateVertexMacros(vertex_format, Mesh::Instancing::on);
			shader_name_suffix += ut::String("_vf") + ut::Print<ut::uint32>(static_cast<ut::uint32>(vertex_format));

			// alpha mode
			GeometryPass::MeshInstRendering::AlphaMode alpha_mode =
				static_cast<GeometryPass::MeshInstRendering::AlphaMode>(
					GeometryPass::MeshInstRendering::ShaderGrid::GetCoordinate<GeometryPass::MeshInstRendering::alpha_mode_column>(i));
			const bool alpha_test = alpha_mode == GeometryPass::MeshInstRendering::AlphaMode::alpha_test;
			macro.name = "ALPHA_TEST";
			macro.value = alpha_test ? "1" : "0";
			macros.Add(macro);
			shader_name_suffix += alpha_test ? "_at_on" : "_at_off";

			// compile shaders
			const ut::String shader_name_prefix = "mesh_gpass_";
			ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::Stage::vertex, shader_name_prefix + "vs" + shader_name_suffix,
			                                                            "VS", "mesh.hlsl", macros);
			ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel, shader_name_prefix + "ps" + shader_name_suffix,
			                                                            "PS", "mesh.hlsl", macros);

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
Shader DeferredShading::CreateLightPassShader(Light::SourceType source_type,
                                              LightPass::IblPreset ibl_preset)
{
	Shader::Macros macros;
	Shader::MacroDefinition macro;

	macro.name = "LIGHT_PASS";
	macro.value = "1";
	macros.Add(ut::Move(macro));

	// light type
	const char* light_type_str;
	const char* light_sufix;
	switch (source_type)
	{
	case Light::SourceType::directional:
		light_type_str = "DIRECTIONAL_LIGHT";
		light_sufix = "directional";
		break;
	case Light::SourceType::point:
		light_type_str = "POINT_LIGHT";
		light_sufix = "point";
		break;
	case Light::SourceType::spot:
		light_type_str = "SPOT_LIGHT";
		light_sufix = "spot";
		break;
	case Light::SourceType::ambient:
		light_type_str = "AMBIENT_LIGHT";
		light_sufix = "ambient";
		break;
	default: throw ut::Error(ut::error::not_implemented);
	}
	macro.name = light_type_str;
	macro.value = "1";
	macros.Add(ut::Move(macro));

	// ibl preset
	const bool ibl_enabled = ibl_preset == LightPass::IblPreset::on;
	const char* ibl_sufix = ibl_enabled ? "_ibl" : "_noibl";
	macro.name = "IBL";
	macro.value = ibl_enabled ? "1" : "0";
	macros.Add(ut::Move(macro));

	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                            ut::String("deferred_shading_ps_") + light_sufix + ibl_sufix,
	                                                            "PS",
	                                                            "deferred_shading.hlsl",
	                                                            macros);
	return ps.MoveOrThrow();
}

// Creates all permutations of light pass shaders.
ut::Array<Shader> DeferredShading::CreateLightPassShaders()
{
	ut::Array<Shader> shaders;

	constexpr size_t light_permutation_count = LightPass::Grid::size;
	for (size_t i = 0; i < light_permutation_count; i++)
	{
		const size_t source_type = LightPass::Grid::GetCoordinate<LightPass::light_type_column>(i);
		const size_t ibl_preset = LightPass::Grid::GetCoordinate<LightPass::ibl_column>(i);
		shaders.Add(CreateLightPassShader(static_cast<Light::SourceType>(source_type),
		                                  static_cast<LightPass::IblPreset>(ibl_preset)));
	}

	return shaders;
}

// Creates a pixel shader for the emissive pass.
Shader DeferredShading::CreateEmissiveShader()
{
	Shader::Macros macros;
	Shader::MacroDefinition macro;

	macro.name = "EMISSIVE_PASS";
	macro.value = "1";
	macros.Add(ut::Move(macro));

	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                            "deferred_emissive_ps",
	                                                            "PS",
	                                                            "deferred_shading.hlsl",
	                                                            macros);
	return ps.MoveOrThrow();
}

// Creates a pixel shader for the image based lighting.
Shader DeferredShading::CreateIblShader(ut::uint32 ibl_mip_count)
{
	Shader::Macros macros;
	Shader::MacroDefinition macro;

	macro.name = "IBL_PASS";
	macro.value = "1";
	macros.Add(ut::Move(macro));

	macro.name = "IBL";
	macro.value = "1";
	macros.Add(ut::Move(macro));

	macro.name = "IBL_MIP_COUNT";
	macro.value = ut::Print(ibl_mip_count);
	macros.Add(ut::Move(macro));

	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                            "deferred_ibl_ps",
	                                                            "PS",
	                                                            "deferred_shading.hlsl",
	                                                            macros);
	return ps.MoveOrThrow();
}

// Creates a render pass for the g-buffer.
RenderPass DeferredShading::CreateMeshInstGeometryPass()
{
	RenderTargetSlot depth_slot(tools.formats.depth_stencil,
	                            RenderTargetSlot::LoadOperation::clear,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	RenderTargetSlot base_color_slot(tools.formats.gbuffer_base_color,
	                                 RenderTargetSlot::LoadOperation::clear,
	                                 RenderTargetSlot::StoreOperation::save,
	                                 false);
	RenderTargetSlot normal_slot(tools.formats.gbuffer_normal,
	                             RenderTargetSlot::LoadOperation::clear,
	                             RenderTargetSlot::StoreOperation::save,
	                             false);
	RenderTargetSlot emissive_slot(tools.formats.gbuffer_emissive,
	                               RenderTargetSlot::LoadOperation::clear,
	                               RenderTargetSlot::StoreOperation::save,
	                               false);

	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(base_color_slot);
	color_slots.Add(normal_slot);
	color_slots.Add(emissive_slot);

	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot).MoveOrThrow();
}

// Creates a render pass for the shading techniques.
RenderPass DeferredShading::CreateLightPass()
{
	RenderTargetSlot depth_slot(tools.formats.depth_stencil,
	                            RenderTargetSlot::LoadOperation::extract,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	RenderTargetSlot color_slot(tools.formats.light_buffer,
	                            RenderTargetSlot::LoadOperation::clear,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot).MoveOrThrow();
}

// Creates a pipeline state to render geometry to the g-buffer.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateMeshInstGPassPipeline(Mesh::VertexFormat vertex_format,
                                                                                  Mesh::PolygonMode polygon_mode,
                                                                                  GeometryPass::MeshInstRendering::AlphaMode alpha_mode,
                                                                                  GeometryPass::MeshInstRendering::CullMode cull_mode,
                                                                                  GeometryPass::MeshInstRendering::StencilMode stencil_mode)
{
	const size_t shader_id = GeometryPass::MeshInstRendering::ShaderGrid::GetId(static_cast<size_t>(vertex_format),
	                                                                            static_cast<size_t>(alpha_mode));
	BoundShader& shader = mesh_inst_gpass_shader[shader_id];

	const ut::uint32 stencil_mask = stencil_mode == GeometryPass::MeshInstRendering::StencilMode::opaque_and_highlighted ?
	                                (static_cast<ut::uint32>(StencilReference::opaque) | static_cast<ut::uint32>(StencilReference::highlight)) :
	                                 static_cast<ut::uint32>(StencilReference::opaque);

	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, shader.GetShader(Shader::Stage::vertex));
	info.SetShader(Shader::Stage::pixel, shader.GetShader(Shader::Stage::pixel));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format, polygon_mode, Mesh::Instancing::on);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = true;
	info.depth_stencil_state.depth_compare_op = compare::Operation::less;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::replace;
	info.depth_stencil_state.back.compare_mask = stencil_mask;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = Mesh::GetRasterizerPolygonMode(polygon_mode);
	info.rasterization_state.cull_mode = cull_mode == GeometryPass::MeshInstRendering::CullMode::back ?
	                                                  RasterizationState::CullMode::back :
	                                                  RasterizationState::CullMode::off;
	info.rasterization_state.line_width = 1.0f;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // base color
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // normal
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // emissive
	return tools.device.CreatePipelineState(ut::Move(info), geometry_pass);
}

// Creates a pipeline state to apply lighting.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateLightPassPipeline(Light::SourceType source_type,
                                                                              LightPass::IblPreset ibl_preset)
{
	PipelineState::Info info;
	const size_t shader_id = LightPass::Grid::GetId(static_cast<size_t>(ibl_preset),
	                                                static_cast<size_t>(source_type));
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, light_shader[shader_id]);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.compare_mask = static_cast<ut::uint32>(StencilReference::opaque);
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = static_cast<ut::uint32>(StencilReference::opaque);
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateAdditiveBlending());
	return tools.device.CreatePipelineState(ut::Move(info), light_pass);
}

// Creates all possible geometry pass pipeline permutations for a mesh instance.
ut::Array<PipelineState> DeferredShading::CreateMeshInstGPassPipelinePermutations()
{
	ut::Array<PipelineState> pipelines;

	constexpr ut::uint32 mesh_inst_pipeline_count = static_cast<ut::uint32>(GeometryPass::MeshInstRendering::PipelineGrid::size);
	for (ut::uint32 i = 0; i < mesh_inst_pipeline_count; i++)
	{
		const size_t vertex_format = GeometryPass::MeshInstRendering::PipelineGrid::GetCoordinate<GeometryPass::MeshInstRendering::vertex_format_column>(i);
		const size_t alpha_mode = GeometryPass::MeshInstRendering::PipelineGrid::GetCoordinate<GeometryPass::MeshInstRendering::alpha_mode_column>(i);
		const size_t cull_mode = GeometryPass::MeshInstRendering::PipelineGrid::GetCoordinate<GeometryPass::MeshInstRendering::cull_mode_column>(i);
		const size_t stencil_mode = GeometryPass::MeshInstRendering::PipelineGrid::GetCoordinate<GeometryPass::MeshInstRendering::stencil_mode_column>(i);
		const size_t polygon_mode = GeometryPass::MeshInstRendering::PipelineGrid::GetCoordinate<GeometryPass::MeshInstRendering::polygon_mode_column>(i);

		ut::Result<PipelineState, ut::Error> pipeline = CreateMeshInstGPassPipeline(static_cast<Mesh::VertexFormat>(vertex_format),
		                                                                            static_cast<Mesh::PolygonMode>(polygon_mode),
		                                                                            static_cast<GeometryPass::MeshInstRendering::AlphaMode>(alpha_mode),
		                                                                            static_cast<GeometryPass::MeshInstRendering::CullMode>(cull_mode),
		                                                                            static_cast<GeometryPass::MeshInstRendering::StencilMode>(stencil_mode));
		if (!pipeline)
		{
			throw ut::Error(pipeline.MoveAlt());
		}

		if (!pipelines.Add(pipeline.Move()))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return pipelines;
}

// Creates all possible light pass pipeline permutations for a mesh instance.
ut::Array<PipelineState> DeferredShading::CreateLightPassPipelinePermutations()
{
	ut::Array<PipelineState> pipelines;

	constexpr size_t light_permutation_count = LightPass::Grid::size;
	for (ut::uint32 i = 0; i < light_permutation_count; i++)
	{
		const size_t source_type = LightPass::Grid::GetCoordinate<LightPass::light_type_column>(i);
		const size_t ibl_preset = LightPass::Grid::GetCoordinate<LightPass::ibl_column>(i);
		ut::Result<PipelineState, ut::Error> pipeline = CreateLightPassPipeline(static_cast<Light::SourceType>(source_type),
		                                                                        static_cast<LightPass::IblPreset>(ibl_preset));
		if (!pipeline)
		{
			throw ut::Error(pipeline.MoveAlt());
		}

		if (!pipelines.Add(pipeline.Move()))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	return pipelines;
}

// Creates a pipeline state to apply emissive lighting.
PipelineState DeferredShading::CreateEmissivePipeline()
{
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, emissive_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.compare_mask = static_cast<ut::uint32>(StencilReference::opaque);
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = static_cast<ut::uint32>(StencilReference::opaque);
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateAdditiveBlending());
	return tools.device.CreatePipelineState(ut::Move(info), light_pass).MoveOrThrow();
}

// Creates a pipeline state to apply image based lighting.
PipelineState DeferredShading::CreateIblPipeline()
{
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, ibl_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.compare_mask = static_cast<ut::uint32>(StencilReference::opaque);
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = static_cast<ut::uint32>(StencilReference::opaque);
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateAdditiveBlending());
	return tools.device.CreatePipelineState(ut::Move(info), light_pass).MoveOrThrow();
}

// Connects all descriptor sets to the corresponding shaders.
void DeferredShading::ConnectDescriptors()
{
	// g-pass shaders
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	gpass_mesh_inst_desc_set.Resize(thread_count);
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		gpass_mesh_inst_desc_set[i].Connect(mesh_inst_gpass_shader.GetFirst());
	}

	// lightpass descriptor is the same for all source types except ambient
	size_t shader_id = LightPass::Grid::GetId(static_cast<size_t>(LightPass::IblPreset::on),
	                                          static_cast<size_t>(Light::SourceType::point));
	light_pass_desc_set.Connect(light_shader[shader_id]);

	// ambient pass has separate descriptor set (uses occlusion map)
	shader_id = LightPass::Grid::GetId(static_cast<size_t>(LightPass::IblPreset::off),
	                                   static_cast<size_t>(Light::SourceType::ambient));
	ambient_pass_desc_set[static_cast<size_t>(LightPass::IblPreset::off)].Connect(light_shader[shader_id]);
	shader_id = LightPass::Grid::GetId(static_cast<size_t>(LightPass::IblPreset::on),
	                                   static_cast<size_t>(Light::SourceType::ambient));
	ambient_pass_desc_set[static_cast<size_t>(LightPass::IblPreset::on)].Connect(light_shader[shader_id]);

	// emissive pass
	emissive_pass_desc_set.Connect(emissive_shader);

	// IBL pass
	ibl_desc_set.Connect(ibl_shader);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//