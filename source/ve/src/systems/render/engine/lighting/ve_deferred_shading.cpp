//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/units/ve_render_view.h"
#include "systems/render/engine/lighting/ve_deferred_shading.h"
#include "systems/render/engine/policy/ve_render_model_policy.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
DeferredShading::DeferredShading(Toolset& toolset) : tools(toolset)
                                                   , model_gpass_shader(CreateModelGPassShader())
                                                   , light_shader{ CreateLightPassShader(Light::source_directional),
                                                                   CreateLightPassShader(Light::source_point),
                                                                   CreateLightPassShader(Light::source_spot) }
{
	gpass_desc_set.Connect(model_gpass_shader);
	lightpass_desc_set.Connect(light_shader[Light::source_directional]);
}

// Creates deferred shading (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param light_buffer - reference to the light buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @return - a new DeferredShading::ViewData object or error if failed.
ut::Result<DeferredShading::ViewData, ut::Error> DeferredShading::CreateViewData(Target& depth_stencil,
                                                                                 Target& light_buffer,
                                                                                 ut::uint32 width,
                                                                                 ut::uint32 height)
{
	Target::Info info;
	info.type = Image::type_2D;
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

	// render pass
	ut::Result<RenderPass, ut::Error> geometry_pass = CreateGeometryPass(depth_stencil.GetInfo().format);
	if (!geometry_pass)
	{
		return ut::MakeError(geometry_pass.MoveAlt());
	}

	// light pass
	ut::Result<RenderPass, ut::Error> light_pass = CreateLightPass(light_buffer.GetInfo().format);
	if (!light_pass)
	{
		return ut::MakeError(light_pass.MoveAlt());
	}

	// geometry pass framebuffer
	ut::Array< ut::Ref<Target> > color_targets;
	color_targets.Add(diffuse.Get());
	color_targets.Add(normal.Get());
	ut::Result<Framebuffer, ut::Error> gpass_framebuffer = tools.device.CreateFramebuffer(geometry_pass.Get(),
	                                                                                      ut::Move(color_targets),
	                                                                                      depth_stencil);
	if (!gpass_framebuffer)
	{
		return ut::MakeError(gpass_framebuffer.MoveAlt());
	}

	// light pass framebuffer
	color_targets.Empty();
	color_targets.Add(light_buffer);
	ut::Result<Framebuffer, ut::Error> light_framebuffer = tools.device.CreateFramebuffer(light_pass.Get(),
	                                                                                      ut::Move(color_targets));
	if (!light_framebuffer)
	{
		return ut::MakeError(light_framebuffer.MoveAlt());
	}

	// gpass pipeline state
	ut::Result<PipelineState, ut::Error> gpass_model_pipeline = CreateModelGPassPipeline(geometry_pass.Get(),
	                                                                                     width, height);
	if (!gpass_model_pipeline)
	{
		return ut::MakeError(gpass_framebuffer.MoveAlt());
	}

	// light pass pipeline state
	ut::Result<PipelineState, ut::Error> lightpass_pipeline[Light::source_type_count] =
	{
		CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_directional),
		CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_point),
		CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_spot)
	};

	for (size_t i = 0; i < Light::source_type_count; i++)
	{
		if (!lightpass_pipeline[i])
		{
			return ut::MakeError(lightpass_pipeline[i].MoveAlt());
		}
	}

	DeferredShading::ViewData data =
	{
		diffuse.Move(),
		normal.Move(),
		geometry_pass.Move(),
		light_pass.Move(),
		gpass_framebuffer.Move(),
		light_framebuffer.Move(),
		gpass_model_pipeline.Move(),
		{
			lightpass_pipeline[Light::source_directional].Move(),
			lightpass_pipeline[Light::source_point].Move(),
			lightpass_pipeline[Light::source_spot].Move()
		}
	};

	return ut::Move(data);
}

// Creates shaders for rendering geometry to the g-buffer.
BoundShader DeferredShading::CreateModelGPassShader()
{
	Shader::Macros macros;
	Shader::MacroDefinition batch_size;
	batch_size.name = "BATCH_SIZE";
	batch_size.value = ut::Print(ModelBatcher::CalculateBatchSize(tools.device));
	macros.Add(ut::Move(batch_size));

	ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::vertex, "geometry_pass_vs", "VS", "geometry_pass.hlsl", macros);
	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel, "geometry_pass_ps", "PS", "geometry_pass.hlsl", macros);
	return BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow());
}

// Creates a shader for the lighting pass.
Shader DeferredShading::CreateLightPassShader(Light::SourceType source_type)
{
	const char* light_type_str;
	switch (source_type)
	{
	case Light::source_directional: light_type_str = "DIRECTIONAL_LIGHT"; break;
	case Light::source_point: light_type_str = "POINT_LIGHT"; break;
	case Light::source_spot: light_type_str = "SPOT_LIGHT"; break;
	default: throw ut::Error(ut::error::not_implemented);
	}

	Shader::Macros macros;
	Shader::MacroDefinition source_type_macro;
	source_type_macro.name = light_type_str;
	source_type_macro.value = "1";
	macros.Add(ut::Move(source_type_macro));

	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
	                                                            "deferred_shading_ps",
	                                                            "PS",
	                                                            "deferred_shading.hlsl",
	                                                            macros);
	return ps.MoveOrThrow();
}

// Creates a render pass for the g-buffer.
ut::Result<RenderPass, ut::Error> DeferredShading::CreateGeometryPass(pixel::Format depth_stencil_format)
{
	RenderTargetSlot depth_slot(depth_stencil_format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	RenderTargetSlot color_slot(skGBufferFormat, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);

	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot); // diffuse
	color_slots.Add(color_slot); // normal

	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot);
}

// Creates a render pass for the shading techniques.
ut::Result<RenderPass, ut::Error> DeferredShading::CreateLightPass(pixel::Format light_buffer_format)
{
	RenderTargetSlot color_slot(light_buffer_format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots));
}

// Creates a pipeline state to render geometry to the g-buffer.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateModelGPassPipeline(RenderPass& geometry_pass,
                                                                               ut::uint32 width,
                                                                               ut::uint32 height)
{
	PipelineState::Info info;
	info.stages[Shader::vertex] = model_gpass_shader.stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = model_gpass_shader.stages[Shader::pixel].Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(width),
	                            static_cast<ut::uint32>(height)));
	info.input_assembly_state = tools.rc_mgr.cube->input_assembly_instancing;
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = true;
	info.depth_stencil_state.depth_compare_op = compare::less;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::back_culling;
	info.rasterization_state.line_width = 1.0f;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // diffuse
	info.blend_state.attachments.Add(BlendState::CreateNoBlending()); // normal
	return tools.device.CreatePipelineState(ut::Move(info), geometry_pass);
}

// Creates a pipeline state to apply lighting.
ut::Result<PipelineState, ut::Error> DeferredShading::CreateLightPassPipeline(RenderPass& light_pass,
                                                                              ut::uint32 width,
                                                                              ut::uint32 height,
                                                                              Light::SourceType source_type)
{
	PipelineState::Info info;
	info.stages[Shader::vertex] = tools.shaders.quad_vs;
	info.stages[Shader::pixel] = light_shader[source_type];
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
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateAdditiveBlending());
	return tools.device.CreatePipelineState(ut::Move(info), light_pass);
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

// Renders model units to the g-buffer.
void DeferredShading::BakeModels(Context& context,
                                 DeferredShading::ViewData& data,
                                 Buffer& view_uniform_buffer,
                                 ModelBatcher& batcher)
{
	// extract model policy data
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	ut::Array<Model::DrawCall>& draw_list = batcher.draw_calls;
	Buffer& instance_buffer = batcher.instance_buffer;
	ut::Array<Model::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	const ut::uint32 batch_size = batcher.GetBatchSize();

	// begin a render pass
	const Framebuffer::Info& fb_info = data.geometry_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.geometry_pass, data.geometry_framebuffer, render_area, ut::Color<4>(0.11f, 0.11f, 0.11f, 1.0f), 1.0f);
	context.BindPipelineState(data.model_pipeline);

	// set common uniforms
	gpass_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	gpass_desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);

	// variables tracking if something changes between iterations
	Map* prev_diffuse_ptr = nullptr;
	Map* prev_normal_ptr = nullptr;
	Map* prev_material_ptr = nullptr;
	Buffer* prev_vertex_buffer = nullptr;
	Buffer* prev_index_buffer = nullptr;
	ut::uint32 prev_index_offset = 0;
	ut::uint32 prev_index_count = 0;
	ut::uint32 prev_batch_id = static_cast<ut::uint32>(batches.GetNum());

	// number of elements to draw at once
	ut::uint32 instance_count = 0;

	// iterate primitive groups and try to merge them in the
	// least possible amount of draw calls
	const ut::uint32 count = static_cast<ut::uint32>(draw_list.GetNum());
	for (ut::uint32 i = 0; i < count; i++)
	{
		// calculate batch id
		const ut::uint32 batch_id = i / batch_size;
		Model::Batch& batch = batches[batch_id];

		// extract material
		Model::DrawCall& dc = draw_list[i];
		Mesh& mesh = dc.model.mesh.Get();
		Mesh::Subset& subset = mesh.subsets[dc.subset_id];
		Material& material = subset.material;

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
		const bool state_changed = shader_rc_changed ||
		                           vertex_buffer_changed ||
		                           index_buffer_changed ||
		                           indices_changed;

		// draw previous instance group
		if (state_changed && i != 0)
		{
			PerformModelDrawCall(context, index_buffer,
			                     index_offset, index_count,
			                     instance_count, i, batch_size);
			instance_count = 0;
		}
		else
		{
			instance_count++;
		}

		// bind descriptors
		if (shader_rc_changed)
		{
			gpass_desc_set.transform_ub.BindUniformBuffer(batch.transform);
			gpass_desc_set.material_ub.BindUniformBuffer(batch.material);
			gpass_desc_set.diffuse.BindImage(material.diffuse.Get());
			gpass_desc_set.normal.BindImage(material.normal.Get());
			gpass_desc_set.material.BindImage(material.material.Get());
			context.BindDescriptorSet(gpass_desc_set);

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
		if ( i == count - 1)
		{
			PerformModelDrawCall(context, index_buffer,
			                     index_offset, index_count,
			                     instance_count, i, batch_size);
		}
	}

	context.EndRenderPass();
}

// Applies lighting techniques to the provided target.
void DeferredShading::Shade(Context& context,
                            DeferredShading::ViewData& data,
                            Buffer& view_uniform_buffer,
                            Image& depth_buffer,
                            Light::Sources& lights)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();

	// begin a render pass
	const Framebuffer::Info& fb_info = data.light_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.light_pass, data.light_framebuffer, render_area, ut::Color<4>(0));

	// set shader resources
	lightpass_desc_set.view_ub.BindUniformBuffer(view_uniform_buffer);
	lightpass_desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);
	lightpass_desc_set.depth.BindImage(depth_buffer);
	lightpass_desc_set.diffuse.BindImage(data.diffuse.GetImage());
	lightpass_desc_set.normal.BindImage(data.normal.GetImage());

	// set quad vertex buffer
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->vertex_buffer, 0);

	// directional lights
	const size_t directional_light_count = lights.directional.GetNum();
	if (directional_light_count > 0)
	{
		context.BindPipelineState(data.light_pipeline[Light::source_directional]);
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
	const size_t point_light_count = lights.point.GetNum();
	if (point_light_count > 0)
	{
		context.BindPipelineState(data.light_pipeline[Light::source_point]);
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
	const size_t spot_light_count = lights.spot.GetNum();
	if (spot_light_count > 0)
	{
		context.BindPipelineState(data.light_pipeline[Light::source_spot]);
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

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//