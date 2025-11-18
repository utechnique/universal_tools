//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_tone_mapping.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)

//----------------------------------------------------------------------------//
// SRGB converter constructor.
ClampToneMapper::ClampToneMapper(Toolset& toolset,
                                 RenderPass& postprocess_pass) : tools(toolset)
                                                               , fullscreen_quad(tools.rc_mgr.fullscreen_quad->subsets.GetFirst())
                                                               , pass(postprocess_pass)
                                                               , pixel_shader(LoadShader())
                                                               , pipeline_state(CreatePipelineState())
{}

// Returns compiled mapping pixel shader.
Shader ClampToneMapper::LoadShader()
{
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                                "tone_map_linear",
	                                                                "ToneMapClamp",
	                                                                "tone_mapping.hlsl");
	return shader.MoveOrThrow();
}

// Creates a pipeline state for this tonemapper effect.
PipelineState ClampToneMapper::CreatePipelineState()
{
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.shaders.quad_vs);
	info.SetShader(Shader::Stage::pixel, pixel_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(ut::Move(info), pass).MoveOrThrow();
}

// Creates per-view data.
//    @return - a new LinearToneMapper::ViewData object or error if failed.
ut::Result<ClampToneMapper::ViewData, ut::Error> ClampToneMapper::CreateViewData()
{
	// create final data object
	ClampToneMapper::ViewData data;

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
//    @param source - reference to the source image.
//    @return - reference to the postprocess slot used for tone mapping.
SwapSlot& ClampToneMapper::Apply(SwapManager& swap_mgr,
                                 Context& context,
                                 ViewData& data,
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
	context.BindPipelineState(pipeline_state);
	context.BindDescriptorSet(data.desc_set);
	context.BindVertexBuffer(fullscreen_quad.vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);
	context.EndRenderPass();

	return slot.Get();
}

//----------------------------------------------------------------------------//
// Tone mapper (per-view) data constructor.
ToneMapper::ViewData::ViewData(ClampToneMapper::ViewData data) : clamp_mapper_data(ut::Move(data))
{}

// Tone mapper constructor.
ToneMapper::ToneMapper(Toolset& toolset,
                       RenderPass& postprocess_pass) : tools(toolset)
                                                     , clamp_mapper(toolset, postprocess_pass)
{}

// Creates tone mapping (per-view) data.
//    @return - a new ToneMapper::ViewData object or error if failed.
ut::Result<ToneMapper::ViewData, ut::Error> ToneMapper::CreateViewData()
{
	// rgb to srgb converter
	ut::Result<ClampToneMapper::ViewData, ut::Error> rgb_to_srgb_data = clamp_mapper.CreateViewData();
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
//    @param source - reference to the source image.
//    @param parameters - reference to the ToneMapper::Parameters object
//                        containing parameters for the tone mapping effect.
//    @return - optional reference to the postprocess slot used for tone
//              mapping.
ut::Optional<SwapSlot&> ToneMapper::Apply(SwapManager& swap_mgr,
                                          Context& context,
                                          ViewData& data,
                                          Image& source,
                                          const Parameters& parameters)
{
	if (!parameters.enabled)
	{
		return ut::Optional<SwapSlot&>();
	}

	return clamp_mapper.Apply(swap_mgr, context, data.clamp_mapper_data, source);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//