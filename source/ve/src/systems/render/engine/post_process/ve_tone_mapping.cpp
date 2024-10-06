//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_tone_mapping.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)

//----------------------------------------------------------------------------//
// SRGB converter (per-view) data constructor.
ClampToneMapper::ViewData::ViewData(PipelineState in_pipeline) : pipeline_state(ut::Move(in_pipeline))
{}

// SRGB converter constructor.
ClampToneMapper::ClampToneMapper(Toolset& toolset) : tools(toolset)
                                                     , pixel_shader(LoadShader())
{}

// Returns compiled mapping pixel shader.
Shader ClampToneMapper::LoadShader()
{
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::pixel,
	                                                                "tone_map_linear",
	                                                                "ToneMapClamp",
	                                                                "tone_mapping.hlsl");
	return shader.MoveOrThrow();
}

// Creates per-view data.
//    @param postprocess_pass - render pass that will be used for mapping.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @return - a new LinearToneMapper::ViewData object or error if failed.
ut::Result<ClampToneMapper::ViewData, ut::Error> ClampToneMapper::CreateViewData(RenderPass& postprocess_pass,
                                                                                   ut::uint32 width,
                                                                                   ut::uint32 height)
{
	PipelineState::Info info;
	info.stages[Shader::vertex] = tools.shaders.quad_vs;
	info.stages[Shader::pixel] = pixel_shader;
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            width, height));
	info.input_assembly_state = tools.rc_mgr.fullscreen_quad->input_assembly;
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::never;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	ut::Result<PipelineState, ut::Error> pipeline_state = tools.device.CreatePipelineState(ut::Move(info),
	                                                                                       postprocess_pass);
	if (!pipeline_state)
	{
		return ut::MakeError(pipeline_state.MoveAlt());
	}

	// create final data object
	ClampToneMapper::ViewData data(pipeline_state.Move());

	// connect descriptors
	data.desc_set.Connect(pixel_shader);

	// success
	return data;
}

// Performs hdr to ldr conversion.
//    @param swap_mgr - reference to the post-process swap manager.
//    @param context - reference to the rendering context.
//    @param data - reference to the LinearToneMapper::ViewData object containing
//                  converter-specific resources.
//    @param pass - reference to the render pass with one color attachment
//                  and no depth.
//    @param source - reference to the source image.
//    @return - reference to the postprocess slot used for tone mapping.
SwapSlot& ClampToneMapper::Apply(SwapManager& swap_mgr,
                                 Context& context,
                                 ViewData& data,
                                 RenderPass& pass,
                                 Image& source)
{
	ut::Optional<SwapSlot&> slot = swap_mgr.Swap();
	UT_ASSERT(slot.HasValue());
	const Framebuffer::Info& fb_info = slot->color_only_framebuffer.GetInfo();

	// set shader resources
	data.desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);
	data.desc_set.tex2d.BindImage(source);

	// draw quad
	const ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(pass,
	                        slot->color_only_framebuffer,
	                        render_area,
	                        ut::Color<4>(0), 1.0f);
	context.BindPipelineState(data.pipeline_state);
	context.BindDescriptorSet(data.desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->vertex_buffer, 0);
	context.Draw(6, 0);
	context.EndRenderPass();

	return slot.Get();
}

//----------------------------------------------------------------------------//
// Tone mapper (per-view) data constructor.
ToneMapper::ViewData::ViewData(ClampToneMapper::ViewData data) : clamp_mapper_data(ut::Move(data))
{}

// Tone mapper constructor.
ToneMapper::ToneMapper(Toolset& toolset) : tools(toolset)
                                         , clamp_mapper(toolset)
{}

// Creates tone mapping (per-view) data.
//    @param postprocess_pass - render pass that will be used for fxaa.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @return - a new ToneMapper::ViewData object or error if failed.
ut::Result<ToneMapper::ViewData, ut::Error> ToneMapper::CreateViewData(RenderPass& postprocess_pass,
                                                                       ut::uint32 width,
                                                                       ut::uint32 height)
{
	// rgb to srgb converter
	ut::Result<ClampToneMapper::ViewData, ut::Error> rgb_to_srgb_data = clamp_mapper.CreateViewData(postprocess_pass,
	                                                                                                width, height);
	if (!rgb_to_srgb_data)
	{
		return ut::MakeError(rgb_to_srgb_data.MoveAlt());
	}

	return ToneMapper::ViewData(rgb_to_srgb_data.Move());
}

// Performs tone mapping.
//    @param swap_mgr - reference to the post-process swap manager.
//    @param context - reference to the rendering context.
//    @param data - reference to the ToneMapper::ViewData object containing
//                  tonemap-specific resources.
//    @param pass - reference to the render pass with one color attachment
//                  and no depth.
//    @param source - reference to the source image.
//    @param parameters - reference to the ToneMapper::Parameters object
//                        containing parameters for the tone mapping effect.
//    @return - optional reference to the postprocess slot used for tone
//              mapping.
ut::Optional<SwapSlot&> ToneMapper::Apply(SwapManager& swap_mgr,
                                          Context& context,
                                          ViewData& data,
                                          RenderPass& pass,
                                          Image& source,
                                          const Parameters& parameters)
{
	if (!parameters.enabled)
	{
		return ut::Optional<SwapSlot&>();
	}

	return clamp_mapper.Apply(swap_mgr, context, data.clamp_mapper_data, pass, source);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//