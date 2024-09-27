//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_view.h"
#include "systems/render/engine/lighting/ve_deferred_shading.h"
#include "systems/render/engine/policy/ve_render_model_policy.h"
#include "systems/render/engine/ve_render_stencil_ref.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
DeferredShading::DeferredShading(Toolset& toolset,
                                 ut::uint32 ibl_mip_count) : tools(toolset)
                                                           , model_gpass_shader(CreateModelGPassShader())
                                                           , ibl_shader(CreateIblShader(ibl_mip_count))
{
	constexpr size_t light_permutation_count = LightPass::Grid::size;
	for (size_t i = 0; i < light_permutation_count; i++)
	{
		const size_t source_type = LightPass::Grid::GetCoordinate<LightPass::light_type_column>(i);
		const size_t ibl_preset = LightPass::Grid::GetCoordinate<LightPass::ibl_column>(i);
		light_shader.Add(CreateLightPassShader(static_cast<Light::SourceType>(source_type),
		                                       static_cast<LightPass::IblPreset>(ibl_preset)));
	}

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
	const pixel::Format depth_stencil_format = depth_stencil.GetInfo().format;
	const ut::uint32 face_count = is_cube ? 6 : 1;

	// target info
	Target::Info info;
	info.type = is_cube ? Image::type_cube : Image::type_2D;
	info.format = skGBufferFormat;
	info.usage = Target::Info::usage_color;
	info.mip_count = 1;
	info.width = width;
	info.height = height;
	info.depth = 1;

	// diffuse
	ut::Result<Target, ut::Error> diffuse = tools.device.CreateTarget(info);
	if (!diffuse)
	{
		return ut::MakeError(diffuse.MoveAlt());
	}

	// normal
	ut::Result<Target, ut::Error> normal = tools.device.CreateTarget(info);
	if (!normal)
	{
		return ut::MakeError(normal.MoveAlt());
	}

	// depth
	info.format = depth_stencil_format;
	info.usage = Target::Info::usage_depth;
	ut::Result<Target, ut::Error> depth = tools.device.CreateTarget(info);
	if (!depth)
	{
		return ut::MakeError(depth.MoveAlt());
	}

	// render pass
	ut::Result<RenderPass, ut::Error> geometry_pass = CreateModelGeometryPass(depth_stencil_format);
	if (!geometry_pass)
	{
		return ut::MakeError(geometry_pass.MoveAlt());
	}

	// light pass
	ut::Result<RenderPass, ut::Error> light_pass = CreateLightPass(depth_stencil_format,
	                                                               light_buffer.GetInfo().format);
	if (!light_pass)
	{
		return ut::MakeError(light_pass.MoveAlt());
	}

	// geometry pass framebuffer
	ut::Array<Framebuffer> gpass_framebuffer;
	for (ut::uint32 face_id = 0; face_id < face_count; face_id++)
	{
		ut::Array<Framebuffer::Attachment> color_targets;
		color_targets.Add(Framebuffer::Attachment(diffuse.Get(), face_id));
		color_targets.Add(Framebuffer::Attachment(normal.Get(), face_id));
		ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(geometry_pass.Get(),
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
		ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(light_pass.Get(),
		                                                                                ut::Move(color_targets),
		                                                                                Framebuffer::Attachment(depth_stencil, face_id));
		if (!framebuffer)
		{
			return ut::MakeError(framebuffer.MoveAlt());
		}
		light_framebuffer.Add(framebuffer.Move());
	}

	// model gpass pipeline state
	constexpr ut::uint32 model_pipeline_count = static_cast<ut::uint32>(GeometryPass::ModelRendering::PipelineGrid::size);
	ut::Array<PipelineState> model_gpass_pipeline;
	for (ut::uint32 i = 0; i < model_pipeline_count; i++)
	{
		const size_t vertex_format = GeometryPass::ModelRendering::PipelineGrid::GetCoordinate<GeometryPass::ModelRendering::vertex_format_column>(i);
		const size_t alpha_mode = GeometryPass::ModelRendering::PipelineGrid::GetCoordinate<GeometryPass::ModelRendering::alpha_mode_column>(i);
		const size_t cull_mode = GeometryPass::ModelRendering::PipelineGrid::GetCoordinate<GeometryPass::ModelRendering::cull_mode_column>(i);
		const size_t stencil_mode = GeometryPass::ModelRendering::PipelineGrid::GetCoordinate<GeometryPass::ModelRendering::stencil_mode_column>(i);

		ut::Result<PipelineState, ut::Error> pipeline = CreateModelGPassPipeline(geometry_pass.Get(),
		                                                                         width, height,
		                                                                         static_cast<Mesh::VertexFormat>(vertex_format),
		                                                                         static_cast<GeometryPass::ModelRendering::AlphaMode>(alpha_mode),
		                                                                         static_cast<GeometryPass::ModelRendering::CullMode>(cull_mode),
		                                                                         static_cast<GeometryPass::ModelRendering::StencilMode>(stencil_mode));
		if (!pipeline)
		{
			return ut::MakeError(pipeline.MoveAlt());
		}

		if (!model_gpass_pipeline.Add(pipeline.Move()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// light pass pipeline state
	ut::Array<PipelineState> lightpass_pipeline;
	constexpr size_t light_permutation_count = LightPass::Grid::size;
	for (ut::uint32 i = 0; i < light_permutation_count; i++)
	{
		const size_t source_type = LightPass::Grid::GetCoordinate<LightPass::light_type_column>(i);
		const size_t ibl_preset = LightPass::Grid::GetCoordinate<LightPass::ibl_column>(i);
		ut::Result<PipelineState, ut::Error> pipeline = CreateLightPassPipeline(light_pass.Get(),
		                                                                        width, height,
		                                                                        static_cast<Light::SourceType>(source_type),
		                                                                        static_cast<LightPass::IblPreset>(ibl_preset));
		if (!pipeline)
		{
			return ut::MakeError(pipeline.MoveAlt());
		}

		if (!lightpass_pipeline.Add(pipeline.Move()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// ibl reflections pipeline state
	ut::Result<PipelineState, ut::Error> ibl_pipeline = CreateIblPipeline(light_pass.Get(),
	                                                                      width, height);
	if (!ibl_pipeline)
	{
		return ut::MakeError(ibl_pipeline.MoveAlt());
	}

	// secondary command buffers
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	const ut::uint32 secondary_buffer_count = thread_count * (is_cube ? 6 : 1);
	ut::Array<CmdBuffer> gpass_cmd;
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

		gpass_cmd.Add(cmd_buffer.Move());
	}

	DeferredShading::ViewData data =
	{
		diffuse.Move(),
		normal.Move(),
		depth.Move(),
		geometry_pass.Move(),
		light_pass.Move(),
		ut::Move(gpass_framebuffer),
		ut::Move(light_framebuffer),
		ut::Move(model_gpass_pipeline),
		ut::Move(lightpass_pipeline),
		ibl_pipeline.Move(),
		ut::Move(gpass_cmd),
	};

	return ut::Move(data);
}

// Renders scnene to the g-buffer.
void DeferredShading::BakeOpaqueGeometry(Context& context,
                                         Target& depth_stencil,
                                         DeferredShading::ViewData& data,
                                         Buffer& view_uniform_buffer,
                                         ModelBatcher& batcher,
                                         Image::Cube::Face cubeface)
{
	BakeOpaqueModels(context, data, view_uniform_buffer, batcher, cubeface);

	// copy depth to the intermediate buffer
	context.SetTargetState(depth_stencil, Target::Info::state_transfer_src);
	context.SetTargetState(data.depth, Target::Info::state_transfer_dst);
	context.CopyTarget(data.depth, depth_stencil, cubeface, 1);
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
	                                        LightPass::ibl_on :
	                                        LightPass::ibl_off;

	// set the g-buffer as a shader resource
	ut::Ref<Target> transition_targets[] = { data.diffuse, data.normal, data.depth };
	context.SetTargetState<3>(transition_targets, Target::Info::state_resource);

	// begin a render pass
	Framebuffer& light_framebuffer = data.light_framebuffer[cubeface];
	const Framebuffer::Info& fb_info = light_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.light_pass, light_framebuffer, render_area, ut::Color<4>(0));

	// set shader resources
	lightpass_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	lightpass_desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);

	// bind g-buffer
	if (data.depth.GetInfo().type == Image::type_cube)
	{
		lightpass_desc_set.depth.BindCubeFace(data.depth.GetImage(), cubeface);
		lightpass_desc_set.diffuse.BindCubeFace(data.diffuse.GetImage(), cubeface);
		lightpass_desc_set.normal.BindCubeFace(data.normal.GetImage(), cubeface);
	}
	else
	{
		lightpass_desc_set.depth.BindImage(data.depth.GetImage());
		lightpass_desc_set.diffuse.BindImage(data.diffuse.GetImage());
		lightpass_desc_set.normal.BindImage(data.normal.GetImage());
	}

	// set quad vertex buffer
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->vertex_buffer, 0);

	// image based lighting
	if (ibl_cubemap)
	{
		if (data.depth.GetInfo().type == Image::type_cube)
		{
			ibl_desc_set.depth.BindCubeFace(data.depth.GetImage(), cubeface);
			ibl_desc_set.diffuse.BindCubeFace(data.diffuse.GetImage(), cubeface);
			ibl_desc_set.normal.BindCubeFace(data.normal.GetImage(), cubeface);
		}
		else
		{
			ibl_desc_set.depth.BindImage(data.depth.GetImage());
			ibl_desc_set.diffuse.BindImage(data.diffuse.GetImage());
			ibl_desc_set.normal.BindImage(data.normal.GetImage());
		}

		ibl_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
		ibl_desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);
		ibl_desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
		ibl_desc_set.ibl_cubemap.BindImage(ibl_cubemap.Get());
		context.BindPipelineState(data.ibl_pipeline);
		context.BindDescriptorSet(ibl_desc_set);
		context.Draw(6, 0);
	}

	// ambient lights
	const size_t ambient_light_count = lights.ambient.Count();
	if (ambient_light_count > 0)
	{
		const size_t pipeline_id = LightPass::Grid::GetId(ibl_preset, Light::source_ambient);
		context.BindPipelineState(data.light_pipeline[pipeline_id]);
		for (size_t i = 0; i < ambient_light_count; i++)
		{
			AmbientLight& light = lights.ambient[i];
			AmbientLight::FrameData& light_data = light.data->frames[current_frame_id];
			lightpass_desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
			context.BindDescriptorSet(lightpass_desc_set);
			context.Draw(6, 0);
		}
	}

	// directional lights
	const size_t directional_light_count = lights.directional.Count();
	if (directional_light_count > 0)
	{
		const size_t pipeline_id = LightPass::Grid::GetId(ibl_preset, Light::source_directional);
		context.BindPipelineState(data.light_pipeline[pipeline_id]);
		for (size_t i = 0; i < directional_light_count; i++)
		{
			DirectionalLight& light = lights.directional[i];
			DirectionalLight::FrameData& light_data = light.data->frames[current_frame_id];
			lightpass_desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
			context.BindDescriptorSet(lightpass_desc_set);
			context.Draw(6, 0);
		}
	}

	// point lights
	const size_t point_light_count = lights.point.Count();
	if (point_light_count > 0)
	{
		const size_t pipeline_id = LightPass::Grid::GetId(ibl_preset, Light::source_point);
		context.BindPipelineState(data.light_pipeline[pipeline_id]);
		for (size_t i = 0; i < point_light_count; i++)
		{
			PointLight& light = lights.point[i];
			PointLight::FrameData& light_data = light.data->frames[current_frame_id];
			lightpass_desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
			context.BindDescriptorSet(lightpass_desc_set);
			context.Draw(6, 0);
		}
	}

	// spot lights
	const size_t spot_light_count = lights.spot.Count();
	if (spot_light_count > 0)
	{
		const size_t pipeline_id = LightPass::Grid::GetId(ibl_preset, Light::source_spot);
		context.BindPipelineState(data.light_pipeline[pipeline_id]);
		for (size_t i = 0; i < spot_light_count; i++)
		{
			SpotLight& light = lights.spot[i];
			SpotLight::FrameData& light_data = light.data->frames[current_frame_id];
			lightpass_desc_set.light_ub.BindUniformBuffer(light_data.uniform_buffer);
			context.BindDescriptorSet(lightpass_desc_set);
			context.Draw(6, 0);
		}
	}

	context.EndRenderPass();
}

// Renders model units to the g-buffer.
void DeferredShading::BakeOpaqueModels(Context& context,
                                       DeferredShading::ViewData& data,
                                       Buffer& view_uniform_buffer,
                                       ModelBatcher& batcher,
                                       Image::Cube::Face cubeface)
{
	// get the number of available threads
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	UT_ASSERT(thread_count == gpass_model_desc_set.Count());

	// get the number of model drawcalls
	ut::Array<Model::DrawCall>& draw_list = batcher.draw_calls;
	const ut::uint32 dc_count = static_cast<ut::uint32>(draw_list.Count());

	// initialize secondary buffers for a parallel work
	secondary_buffer_cache.Reset();
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		secondary_buffer_cache.Add(data.gpass_cmd[cubeface * thread_count + i]);
		tools.device.ResetCmdBuffer(secondary_buffer_cache.GetLast().Get());
	}

	// begin a render pass
	Framebuffer& geometry_framebuffer = data.geometry_framebuffer[cubeface];
	const Framebuffer::Info& fb_info = geometry_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.geometry_pass, geometry_framebuffer, render_area, ut::Color<4>(0), 1.0f, 0, true);

	// parallelize drawcalls
	const ut::uint32 dc_per_thread = dc_count / thread_count; // drawcalls per thread
	ut::uint32 offset = 0; // offset from the first drawcall in the batcher
	for (ut::uint32 thread_id = 0; thread_id < thread_count; thread_id++)
	{
		ut::uint32 count = dc_per_thread + (thread_id == 0 ? (dc_count % thread_count) : 0);

		auto record_commands = [&, thread_id, offset, count](Context& deferred_context)
		{
			BakeOpaqueModelsJob(deferred_context,
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
			                    data.geometry_pass,
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

// Helper function to draw a model's subset.
inline void PerformModelDrawCall(Context& context,
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

// Renders specified range of models.
void DeferredShading::BakeOpaqueModelsJob(Context& context,
                                          DeferredShading::ViewData& data,
                                          Buffer& view_uniform_buffer,
                                          ModelBatcher& batcher,
                                          ut::uint32 thread_id,
                                          ut::uint32 offset,
                                          ut::uint32 count)
{
	if (count == 0)
	{
		return;
	}

	// extract model policy data
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	ut::Array<Model::DrawCall>& draw_list = batcher.draw_calls;
	Buffer& instance_buffer = batcher.instance_buffer;
	ut::Array<Model::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	const ut::uint32 batch_size = batcher.GetBatchSize();

	// set common uniforms
	GeometryPass::ModelRendering::Descriptors& desc_set = gpass_model_desc_set[thread_id];
	desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);

	// variables tracking if something changes between iterations
	Mesh::VertexFormat prev_vertex_format = Mesh::vertex_format_count;
	GeometryPass::ModelRendering::AlphaMode prev_alpha_mode = GeometryPass::ModelRendering::alpha_mode_count;
	GeometryPass::ModelRendering::CullMode prev_cull_mode = GeometryPass::ModelRendering::cull_mode_count;
	GeometryPass::ModelRendering::StencilMode prev_stencil_mode = GeometryPass::ModelRendering::stencil_mode_count;
	Map* prev_diffuse_ptr = nullptr;
	Map* prev_normal_ptr = nullptr;
	Map* prev_material_ptr = nullptr;
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
		Model::DrawCall& dc = draw_list[i];
		Mesh& mesh = dc.model.mesh.Get();
		Mesh::Subset& subset = mesh.subsets[dc.subset_id];
		Material& material = subset.material;
		const bool is_transparent = material.alpha == Material::alpha_transparent;

		// skip transparent materials
		if (is_transparent)
		{
			PerformModelDrawCall(context, prev_index_buffer,
			                     prev_index_offset, prev_index_count,
			                     instance_count, i - 1, batch_size);
			instance_count = 0;
			continue;
		}

		// calculate batch id
		const ut::uint32 batch_id = i / batch_size;
		Model::Batch& batch = batches[batch_id];

		// material maps
		Map* diffuse_ptr = &material.diffuse.Get();
		Map* normal_ptr = &material.normal.Get();
		Map* material_ptr = &material.material.Get();

		// buffers
		Buffer* vertex_buffer = &mesh.vertex_buffer;
		Buffer* index_buffer = mesh.index_buffer ? &mesh.index_buffer.Get() : nullptr;

		// index count
		const ut::uint32 index_offset = subset.index_offset;
		const ut::uint32 index_count = subset.index_count;

		// check pipeline state
		const Mesh::VertexFormat vertex_format = mesh.vertex_format;
		const GeometryPass::ModelRendering::AlphaMode alpha_mode = material.alpha == Material::alpha_masked ?
		                                                           GeometryPass::ModelRendering::alpha_test :
		                                                           GeometryPass::ModelRendering::alpha_opaque;
		const GeometryPass::ModelRendering::CullMode cull_mode = material.double_sided ?
		                                                         GeometryPass::ModelRendering::cull_none :
		                                                         GeometryPass::ModelRendering::cull_back;
		const GeometryPass::ModelRendering::StencilMode stencil_mode = dc.model.highlighted ?
		                                                               GeometryPass::ModelRendering::stencil_opaque_and_highlighted :
		                                                               GeometryPass::ModelRendering::stencil_opaque;
		bool pipeline_changed = prev_vertex_format != vertex_format ||
		                        prev_cull_mode != cull_mode ||
		                        prev_alpha_mode != alpha_mode ||
		                        prev_stencil_mode != stencil_mode;

		// check if at least one shader resource has changed
		const bool shader_rc_changed = batch_id != prev_batch_id ||
		                               diffuse_ptr != prev_diffuse_ptr ||
		                               normal_ptr != prev_normal_ptr ||
		                               material_ptr != prev_material_ptr;

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
			PerformModelDrawCall(context, prev_index_buffer,
			                     prev_index_offset, prev_index_count,
			                     instance_count, i - 1, batch_size);
			instance_count = 0;
		}
		instance_count++;

		// set pipeline state
		if (pipeline_changed)
		{
			const size_t pipeline_state_id = GeometryPass::ModelRendering::PipelineGrid::GetId(vertex_format,
			                                                                                   alpha_mode,
			                                                                                   cull_mode,
			                                                                                   stencil_mode);
			context.BindPipelineState(data.model_gpass_pipeline[pipeline_state_id]);

			prev_vertex_format = vertex_format;
			prev_alpha_mode = alpha_mode;
			prev_cull_mode = cull_mode;
			prev_stencil_mode = stencil_mode;
		}

		// bind descriptors
		if (shader_rc_changed)
		{
			desc_set.transform_ub.BindUniformBuffer(batch.transform);
			desc_set.material_ub.BindUniformBuffer(batch.material);
			desc_set.diffuse.BindImage(material.diffuse.Get());
			desc_set.normal.BindImage(material.normal.Get());
			desc_set.material.BindImage(material.material.Get());
			context.BindDescriptorSet(desc_set);

			prev_batch_id = batch_id;
			prev_diffuse_ptr = diffuse_ptr;
			prev_normal_ptr = normal_ptr;
			prev_material_ptr = material_ptr;
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
				context.BindIndexBuffer(*index_buffer, 0, mesh.index_type);
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
			PerformModelDrawCall(context, index_buffer,
			                     index_offset, index_count,
			                     instance_count, i, batch_size);
		}
	}
}

// Creates shaders for rendering geometry to the g-buffer.
ut::Array<BoundShader> DeferredShading::CreateModelGPassShader()
{
	const ut::String shader_name_prefix = "model_gpass_";
	ut::Array<BoundShader> shaders;
	const size_t shader_count = GeometryPass::ModelRendering::ShaderGrid::size;
	for (size_t i = 0; i < shader_count; i++)
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
		macro.value = ut::Print(ModelBatcher::CalculateBatchSize(tools.device));
		macros.Add(macro);

		// vertex traits
		Mesh::VertexFormat vertex_format = static_cast<Mesh::VertexFormat>(GeometryPass::ModelRendering::ShaderGrid::GetCoordinate<GeometryPass::ModelRendering::vertex_format_column>(i));
		macros += Mesh::GenerateVertexMacros(vertex_format, true);
		shader_name_suffix += ut::String("_vf") + ut::Print<ut::uint32>(vertex_format);

		// alpha mode
		GeometryPass::ModelRendering::AlphaMode alpha_mode = static_cast<GeometryPass::ModelRendering::AlphaMode>(GeometryPass::ModelRendering::ShaderGrid::GetCoordinate<GeometryPass::ModelRendering::alpha_mode_column>(i));
		const bool alpha_test = alpha_mode == GeometryPass::ModelRendering::alpha_test;
		macro.name = "ALPHA_TEST";
		macro.value = alpha_test ? "1" : "0";
		macros.Add(macro);
		shader_name_suffix += alpha_test ? "_at_on" : "_at_off";

		// compile shaders
		ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex, shader_name_prefix + "vs" + shader_name_suffix,
		                                                            "VS", "model.hlsl", macros);
		ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel, shader_name_prefix + "ps" + shader_name_suffix,
		                                                            "PS", "model.hlsl", macros);
		
		if (!shaders.Add(BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow())))
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
	case Light::source_directional:
		light_type_str = "DIRECTIONAL_LIGHT";
		light_sufix = "directional";
		break;
	case Light::source_point:
		light_type_str = "POINT_LIGHT";
		light_sufix = "point";
		break;
	case Light::source_spot:
		light_type_str = "SPOT_LIGHT";
		light_sufix = "spot";
		break;
	case Light::source_ambient:
		light_type_str = "AMBIENT_LIGHT";
		light_sufix = "ambient";
		break;
	default: throw ut::Error(ut::error::not_implemented);
	}
	macro.name = light_type_str;
	macro.value = "1";
	macros.Add(ut::Move(macro));

	// ibl preset
	const bool ibl_enabled = ibl_preset == LightPass::ibl_on;
	const char* ibl_sufix = ibl_enabled ? "_ibl" : "_noibl";
	macro.name = "IBL";
	macro.value = ibl_enabled ? "1" : "0";
	macros.Add(ut::Move(macro));

	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
	                                                            ut::String("deferred_shading_ps_") + light_sufix + ibl_sufix,
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

	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
	                                                            "deferred_ibl_ps",
	                                                            "PS",
	                                                            "deferred_shading.hlsl",
	                                                            macros);
	return ps.MoveOrThrow();
}

// Creates a render pass for the g-buffer.
ut::Result<RenderPass, ut::Error> DeferredShading::CreateModelGeometryPass(pixel::Format depth_stencil_format)
{
	RenderTargetSlot depth_slot(depth_stencil_format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	RenderTargetSlot color_slot(skGBufferFormat, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);

	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot); // diffuse
	color_slots.Add(color_slot); // normal

	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot);
}

// Creates a render pass for the shading techniques.
ut::Result<RenderPass, ut::Error> DeferredShading::CreateLightPass(pixel::Format depth_stencil_format,
                                                                   pixel::Format light_buffer_format)
{
	RenderTargetSlot depth_slot(depth_stencil_format, RenderTargetSlot::load_extract, RenderTargetSlot::store_save, false);
	RenderTargetSlot color_slot(light_buffer_format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot);
}

// Creates a pipeline state to render geometry to the g-buffer.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateModelGPassPipeline(RenderPass& geometry_pass,
                                                                               ut::uint32 width,
                                                                               ut::uint32 height,
                                                                               Mesh::VertexFormat vertex_format,
                                                                               GeometryPass::ModelRendering::AlphaMode alpha_mode,
                                                                               GeometryPass::ModelRendering::CullMode cull_mode,
                                                                               GeometryPass::ModelRendering::StencilMode stencil_mode)
{
	const size_t shader_id = GeometryPass::ModelRendering::ShaderGrid::GetId(vertex_format, alpha_mode);
	BoundShader& shader = model_gpass_shader[shader_id];

	const ut::uint32 stencil_mask = stencil_mode == GeometryPass::ModelRendering::stencil_opaque_and_highlighted ?
		(stencilref_opaque | stencilref_highlight) : stencilref_opaque;

	PipelineState::Info info;
	info.stages[Shader::vertex] = shader.stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = shader.stages[Shader::pixel].Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = Mesh::CreateIaState(vertex_format, true);
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = true;
	info.depth_stencil_state.depth_compare_op = compare::less;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::replace;
	info.depth_stencil_state.back.compare_mask = stencil_mask;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0xff;
	info.depth_stencil_state.stencil_reference = stencil_mask;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = cull_mode == GeometryPass::ModelRendering::cull_back ?
	                                                  RasterizationState::back_culling :
	                                                  RasterizationState::no_culling;
	info.rasterization_state.line_width = 1.0f;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // diffuse
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // normal
	return tools.device.CreatePipelineState(ut::Move(info), geometry_pass);
}

// Creates a pipeline state to apply lighting.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateLightPassPipeline(RenderPass& light_pass,
                                                                              ut::uint32 width,
                                                                              ut::uint32 height,
                                                                              Light::SourceType source_type,
                                                                              LightPass::IblPreset ibl_preset)
{
	PipelineState::Info info;
	const size_t shader_id = LightPass::Grid::GetId(ibl_preset, source_type);
	info.stages[Shader::vertex] = tools.shaders.quad_vs;
	info.stages[Shader::pixel] = light_shader[shader_id];
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = tools.rc_mgr.fullscreen_quad->input_assembly;
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::keep;
	info.depth_stencil_state.back.compare_mask = stencilref_opaque;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = stencilref_opaque;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateAdditiveBlending());
	return tools.device.CreatePipelineState(ut::Move(info), light_pass);
}

// Creates a pipeline state to apply image based lighting.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateIblPipeline(RenderPass& light_pass,
                                                                        ut::uint32 width,
                                                                        ut::uint32 height)
{
	PipelineState::Info info;
	info.stages[Shader::vertex] = tools.shaders.quad_vs;
	info.stages[Shader::pixel] = ibl_shader;
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = tools.rc_mgr.fullscreen_quad->input_assembly;
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::keep;
	info.depth_stencil_state.back.compare_mask = stencilref_opaque;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = stencilref_opaque;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateAdditiveBlending());
	return tools.device.CreatePipelineState(ut::Move(info), light_pass);
}

// Connects all descriptor sets to the corresponding shaders.
void DeferredShading::ConnectDescriptors()
{
	const ut::uint32 thread_count = static_cast<ut::uint32>(tools.pool.GetThreadCount());
	gpass_model_desc_set.Resize(thread_count);
	for (ut::uint32 i = 0; i < thread_count; i++)
	{
		gpass_model_desc_set[i].Connect(model_gpass_shader.GetFirst());
	}

	ibl_desc_set.Connect(ibl_shader);

	// lightpass descriptor is the same for all source types
	lightpass_desc_set.Connect(light_shader.GetFirst());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//