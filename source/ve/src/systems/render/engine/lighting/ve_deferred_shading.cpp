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
                                                           , light_shader{ { CreateLightPassShader(Light::source_directional, false),
                                                                             CreateLightPassShader(Light::source_point, false),
                                                                             CreateLightPassShader(Light::source_spot, false) },
                                                                           { CreateLightPassShader(Light::source_directional, true),
                                                                             CreateLightPassShader(Light::source_point, true),
                                                                             CreateLightPassShader(Light::source_spot, true) } }
                                                           , ibl_shader(CreateIblShader(ibl_mip_count))
{
	gpass_desc_set.Connect(model_gpass_shader);
	ibl_desc_set.Connect(ibl_shader);
	lightpass_desc_set.Connect(light_shader[ibl_off][Light::source_directional]);
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
	ut::Result<RenderPass, ut::Error> geometry_pass = CreateGeometryPass(depth_stencil_format);
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

	// gpass pipeline state
	ut::Result<PipelineState, ut::Error> gpass_model_pipeline = CreateModelGPassPipeline(geometry_pass.Get(),
	                                                                                     width, height);
	if (!gpass_model_pipeline)
	{
		return ut::MakeError(gpass_model_pipeline.MoveAlt());
	}

	// light pass pipeline state
	ut::Result<PipelineState, ut::Error> lightpass_pipeline[ibl_preset_count][Light::source_type_count] =
	{
		{
			CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_directional, ibl_off),
			CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_point, ibl_off),
			CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_spot, ibl_off)
		},
		{
			CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_directional, ibl_on),
			CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_point, ibl_on),
			CreateLightPassPipeline(light_pass.Get(), width, height, Light::source_spot, ibl_on)
		}
	};

	for (size_t ibl_preset = 0; ibl_preset < ibl_preset_count; ibl_preset++)
	{
		for (size_t light_type = 0; light_type < Light::source_type_count; light_type++)
		{
			if (!lightpass_pipeline[ibl_preset][light_type])
			{
				return ut::MakeError(lightpass_pipeline[ibl_preset][light_type].MoveAlt());
			}
		}
	}

	// gpass pipeline state
	ut::Result<PipelineState, ut::Error> ibl_pipeline = CreateIblPipeline(light_pass.Get(),
	                                                                      width, height);
	if (!ibl_pipeline)
	{
		return ut::MakeError(ibl_pipeline.MoveAlt());
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
		gpass_model_pipeline.Move(),
		{
			{
				lightpass_pipeline[ibl_off][Light::source_directional].Move(),
				lightpass_pipeline[ibl_off][Light::source_point].Move(),
				lightpass_pipeline[ibl_off][Light::source_spot].Move()
			},
			{
				lightpass_pipeline[ibl_on][Light::source_directional].Move(),
				lightpass_pipeline[ibl_on][Light::source_point].Move(),
				lightpass_pipeline[ibl_on][Light::source_spot].Move()
			}
		},
		ibl_pipeline.Move()
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
Shader DeferredShading::CreateLightPassShader(Light::SourceType source_type, bool ibl_enabled)
{
	Shader::Macros macros;
	Shader::MacroDefinition macro;

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
	default: throw ut::Error(ut::error::not_implemented);
	}
	macro.name = light_type_str;
	macro.value = "1";
	macros.Add(ut::Move(macro));

	// ibl preset
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
	macro.name = "IBL_MIP_COUNT";
	macro.value = ut::Print(ibl_mip_count);
	macros.Add(ut::Move(macro));

	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::pixel,
	                                                            "deferred_ibl_ps",
	                                                            "PS",
	                                                            "deferred_ibl.hlsl",
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
ut::Result<RenderPass, ut::Error> DeferredShading::CreateLightPass(pixel::Format depth_stencil_format,
                                                                   pixel::Format light_buffer_format)
{
	RenderTargetSlot depth_slot(depth_stencil_format, RenderTargetSlot::load_extract, RenderTargetSlot::store_dont_care, false);
	RenderTargetSlot color_slot(light_buffer_format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots), depth_slot);
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
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::always;
	info.depth_stencil_state.back.fail_op = StencilOpState::replace;
	info.depth_stencil_state.back.pass_op = StencilOpState::replace;
	info.depth_stencil_state.back.compare_mask = stencilref_opaque;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = stencilref_opaque;
	info.depth_stencil_state.stencil_reference = stencilref_opaque;
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
                                                                              Light::SourceType source_type,
                                                                              IblPreset ibl_preset)
{
	PipelineState::Info info;
	info.stages[Shader::vertex] = tools.shaders.quad_vs;
	info.stages[Shader::pixel] = light_shader[ibl_preset][source_type];
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

// Renders scnene to the g-buffer.
void DeferredShading::BakeGeometry(Context& context,
                                   Target& depth_stencil,
                                   DeferredShading::ViewData& data,
                                   Buffer& view_uniform_buffer,
                                   ModelBatcher& batcher,
                                   Image::Cube::Face cubeface)
{
	BakeModels(context, data, view_uniform_buffer, batcher, cubeface);

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
	const IblPreset ibl_preset = ibl_cubemap ? ibl_on : ibl_off;

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
		ibl_desc_set.ibl_sampler.BindSampler(tools.sampler_cache.linear_wrap);
		ibl_desc_set.ibl_cubemap.BindImage(ibl_cubemap.Get());
		context.BindPipelineState(data.ibl_pipeline);
		context.BindDescriptorSet(ibl_desc_set);
		context.Draw(6, 0);
	}

	// directional lights
	const size_t directional_light_count = lights.directional.GetNum();
	if (directional_light_count > 0)
	{
		context.BindPipelineState(data.light_pipeline[ibl_preset][Light::source_directional]);
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
		context.BindPipelineState(data.light_pipeline[ibl_preset][Light::source_point]);
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
		context.BindPipelineState(data.light_pipeline[ibl_preset][Light::source_spot]);
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
                                 ModelBatcher& batcher,
                                 Image::Cube::Face cubeface)
{
	// extract model policy data
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();
	ut::Array<Model::DrawCall>& draw_list = batcher.draw_calls;
	Buffer& instance_buffer = batcher.instance_buffer;
	ut::Array<Model::Batch>& batches = batcher.frame_data[current_frame_id].batches;
	const ut::uint32 batch_size = batcher.GetBatchSize();

	// begin a render pass
	Framebuffer& geometry_framebuffer = data.geometry_framebuffer[cubeface];
	const Framebuffer::Info& fb_info = geometry_framebuffer.GetInfo();
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(data.geometry_pass, geometry_framebuffer, render_area, ut::Color<4>(0), 1.0f);
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

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//