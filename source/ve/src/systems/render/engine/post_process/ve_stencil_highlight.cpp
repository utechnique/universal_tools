//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_stencil_highlight.h"
#include "systems/render/engine/ve_render_stencil_ref.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Static variables:
const ut::uint32 StencilHighlight::skHighlightRadius = 3;
const float StencilHighlight::skHighlightSigma = skHighlightRadius / 0.2f;
const ut::uint32 StencilHighlight::skLineDistance = 16;
const ut::uint32 StencilHighlight::skLineWidth = 8;

//----------------------------------------------------------------------------//
// Stencil highlighting (per-view) data constructor.
StencilHighlight::ViewData::ViewData(GaussianBlur::ViewData horizontal_blur_data,
                                     GaussianBlur::ViewData vertical_blur_data,
                                     Buffer in_white_color_buffer,
                                     Buffer in_lines_color_buffer,
                                     Buffer in_blend_color_buffer) : horizontal_blur(ut::Move(horizontal_blur_data))
                                                                   , vertical_blur(ut::Move(vertical_blur_data))
                                                                   , white_color_buffer(ut::Move(in_white_color_buffer))
                                                                   , lines_color_buffer(ut::Move(in_lines_color_buffer))
                                                                   , blend_color_buffer(ut::Move(in_blend_color_buffer))
{}

// Stencil highlighting constructor.
StencilHighlight::StencilHighlight(Toolset& toolset,
                                   GaussianBlur& gaussian_blur,
                                   RenderPass& in_color_only_pass,
                                   RenderPass& in_color_and_ds_pass,
                                   RenderPass& in_clear_color_and_ds_pass) : tools(toolset)
                                                                           , fullscreen_quad(tools.rc_mgr.fullscreen_quad->subsets.GetFirst())
                                                                           , blur(gaussian_blur)
                                                                           , fill_shader(LoadFillShader())
                                                                           , line_shader(LoadLineShader())
                                                                           , blend_shader(LoadBlendShader())
                                                                           , blur_shader(gaussian_blur.LoadShader(skHighlightRadius,
                                                                                                                  skHighlightSigma).MoveOrThrow())
                                                                           , color_only_pass(in_color_only_pass)
                                                                           , color_and_ds_pass(in_color_and_ds_pass)
                                                                           , clear_color_and_ds_pass(in_clear_color_and_ds_pass)
                                                                           , fill_pass_pipeline(CreateFillPassPipelineState())
                                                                           , lines_pass_pipeline(CreateLinePassPipelineState())
                                                                           , blend_pass_pipeline(CreateBlendPassPipelineState())
{}

// Returns compiled pixel shader filling a surface with solid color.
Shader StencilHighlight::LoadFillShader()
{
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                                "stencil_highlight_fill_ps",
	                                                                "FillStencilPS",
	                                                                "highlight.hlsl");
	return shader.MoveOrThrow();
}

// Returns compiled pixel shader filling a surface with solid color.
Shader StencilHighlight::LoadLineShader()
{
	Shader::Macros macros;
	Shader::MacroDefinition macro;

	macro.name = "LINE_WIDTH";
	macro.value = ut::Print(skLineWidth);
	macros.Add(ut::Move(macro));

	macro.name = "LINE_DISTANCE";
	macro.value = ut::Print(skLineDistance);
	macros.Add(ut::Move(macro));

	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                                "stencil_highlight_lines_ps",
	                                                                "DrawLinesPS",
	                                                                "highlight.hlsl",
	                                                                ut::Move(macros));
	return shader.MoveOrThrow();
}

// Returns compiled pixel shader blending source texture with
// highlighting mask.
Shader StencilHighlight::LoadBlendShader()
{
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                                "stencil_highlight_blend_ps",
	                                                                "BlendPS",
	                                                                "highlight.hlsl");
	return shader.MoveOrThrow();
}

// Creates highlighting (per-view) data.
//    @return - a new StencilHighlight::ViewData object or error if failed.
ut::Result<StencilHighlight::ViewData, ut::Error> StencilHighlight::CreateViewData()
{
	// create uniform buffers
	ut::Vector<4> white(1.0f);
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::Type::uniform;
	buffer_info.usage = render::memory::Usage::gpu_immutable;
	buffer_info.size = sizeof(ut::Vector<4>);
	buffer_info.data.Resize(buffer_info.size);
	ut::memory::Copy(buffer_info.data.GetAddress(), white.GetData(), buffer_info.size);
	ut::Result<Buffer, ut::Error> white_color_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!white_color_buffer)
	{
		throw ut::Error(white_color_buffer.MoveAlt());
	}
	buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
	ut::Result<Buffer, ut::Error> lines_color_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!lines_color_buffer)
	{
		throw ut::Error(lines_color_buffer.MoveAlt());
	}
	ut::Result<Buffer, ut::Error> blend_color_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!blend_color_buffer)
	{
		throw ut::Error(blend_color_buffer.MoveAlt());
	}

	// create horizontal gaussian blur data
	ut::Result<GaussianBlur::ViewData, ut::Error> horizontal_blur = blur.CreateViewData(color_only_pass,
	                                                                                    blur_shader,
	                                                                                    skHighlightRadius,
	                                                                                    skHighlightSigma);
	if (!horizontal_blur)
	{
		return ut::MakeError(horizontal_blur.MoveAlt());
	}

	// create vertical gaussian blur data
	ut::Result<GaussianBlur::ViewData, ut::Error> vertical_blur = blur.CreateViewData(color_only_pass,
	                                                                                  blur_shader,
	                                                                                  skHighlightRadius,
	                                                                                  skHighlightSigma);
	if (!vertical_blur)
	{
		return ut::MakeError(vertical_blur.MoveAlt());
	}

	// modify blur weights
	const size_t blur_kernel_size = horizontal_blur->weights.Count();
	for (size_t i = 0; i < blur_kernel_size; i++)
	{
		horizontal_blur->weights[i].Z() *= 2.0f;
		vertical_blur->weights[i].Z() *= 2.0f;
	}

	// create final data object
	StencilHighlight::ViewData data(horizontal_blur.Move(),
	                                vertical_blur.Move(),
	                                white_color_buffer.Move(),
	                                lines_color_buffer.Move(),
	                                blend_color_buffer.Move());

	// connect descriptors
	data.fill_desc_set.Connect(fill_shader);
	data.lines_desc_set.Connect(line_shader);
	data.blend_desc_set.Connect(blend_shader);

	// success
	return data;
}

// Performs highlighting.
//    @param swap_mgr - reference to the post-process swap manager.
//    @param context - reference to the rendering context.
//    @param data - reference to the StencilHighlight::ViewData object.
//    @param source - reference to the source image.
// 	  @param parameters - reference to the StencilHighlight::Parameters object
//                        containing parameters for the highlighting effect.
//    @param time_ms - total accumulated time in milliseconds.
//    @return - optional reference to the postprocess slot used for highlighting.
ut::Optional<SwapSlot&> StencilHighlight::Apply(SwapManager& swap_mgr,
                                                Context& context,
                                                ViewData& data,
                                                Image& source,
                                                const Parameters& parameters,
                                                float time_ms)
{
	if (!parameters.enabled)
	{
		return ut::Optional<SwapSlot&>();
	}

	// get post-process render target
	ut::Optional<SwapSlot&> fill_slot = swap_mgr.Swap();
	UT_ASSERT(fill_slot.HasValue());
	const Framebuffer::Info& fb_info = fill_slot->color_and_ds_framebuffer.GetInfo();

	// fill stencil area with white color
	const ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	data.fill_desc_set.ub_color.BindUniformBuffer(data.white_color_buffer);
	context.BeginRenderPass(clear_color_and_ds_pass,
	                        fill_slot->color_and_ds_framebuffer,
	                        render_area,
	                        ut::Color<4>(0), 1.0f);
	context.BindPipelineState(fill_pass_pipeline);
	context.BindDescriptorSet(data.fill_desc_set);
	context.BindVertexBuffer(fullscreen_quad.vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);
	context.EndRenderPass();
	context.SetTargetState(fill_slot->color_target, Target::Info::State::resource);

	// blur image to extend object borders
	ut::Optional<SwapSlot&> hblur_slot = swap_mgr.Swap();
	UT_ASSERT(hblur_slot.HasValue());
	blur.Apply(context,
	           data.horizontal_blur,
	           hblur_slot->color_only_framebuffer,
	           color_only_pass,
	           fill_slot->color_target.GetImage(),
	           GaussianBlur::Direction::horizontal);
	context.SetTargetState(hblur_slot->color_target, Target::Info::State::resource);
	fill_slot->busy = false;
	ut::Optional<SwapSlot&> vblur_slot = swap_mgr.Swap();
	UT_ASSERT(hblur_slot.HasValue());
	blur.Apply(context,
	           data.vertical_blur,
	           vblur_slot->color_only_framebuffer,
	           color_only_pass,
	           hblur_slot->color_target.GetImage(),
	           GaussianBlur::Direction::vertical);
	hblur_slot->busy = false;

	// update line buffer
	ut::Vector<4> lines_data(fb_info.width,
	                         fb_info.height,
	                         CalculateLineVisibility(parameters, time_ms),
	                         CalculateLineOffset(parameters, time_ms));
	ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                                                    data.lines_color_buffer,
	                                                                    lines_data.GetData());
	if (update_ub_error)
	{
		throw update_ub_error.Move();
	}

	// fill stencil area with black background and white animated lines
	data.lines_desc_set.ub_color.BindUniformBuffer(data.lines_color_buffer);
	context.BeginRenderPass(color_and_ds_pass,
	                        vblur_slot->color_and_ds_framebuffer,
	                        render_area,
	                        ut::Color<4>(0), 1.0f);
	context.BindPipelineState(lines_pass_pipeline);
	context.BindDescriptorSet(data.lines_desc_set);
	context.BindVertexBuffer(fullscreen_quad.vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);
	context.EndRenderPass();
	context.SetTargetState(vblur_slot->color_target, Target::Info::State::resource);

	// update blend color buffer
	update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                            data.blend_color_buffer,
	                                            parameters.highlight_color.GetData());
	if (update_ub_error)
	{
		throw update_ub_error.Move();
	}

	// blend source texture with highlighting mask
	ut::Optional<SwapSlot&> blend_slot = swap_mgr.Swap();
	UT_ASSERT(blend_slot.HasValue());
	data.blend_desc_set.ub_color.BindUniformBuffer(data.blend_color_buffer);
	data.blend_desc_set.input_tex2d.BindImage(source);
	data.blend_desc_set.highlight_tex2d.BindImage(vblur_slot->color_target.GetImage());
	data.blend_desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);
	context.BeginRenderPass(color_only_pass,
	                        blend_slot->color_only_framebuffer,
	                        render_area,
	                        ut::Color<4>(0), 1.0f);
	context.BindPipelineState(blend_pass_pipeline);
	context.BindDescriptorSet(data.blend_desc_set);
	context.BindVertexBuffer(fullscreen_quad.vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);
	context.EndRenderPass();
	vblur_slot->busy = false;

	return blend_slot;
}

// Calculates visibility factor for lines animation.
float StencilHighlight::CalculateLineVisibility(const Parameters& parameters,
                                                float time_ms)
{
	const int visibility_period = static_cast<int>(time_ms / parameters.line_visibility_anim_speed_ms);
	float line_visibility = time_ms - static_cast<float>(visibility_period) *
	                        parameters.line_visibility_anim_speed_ms;
	line_visibility /= parameters.line_visibility_anim_speed_ms;

	if (line_visibility < 0.5f)
	{
		line_visibility = line_visibility * 2.0f;
	}
	else
	{
		line_visibility = 1.0f - (line_visibility - 0.5f) * 2.0f;
	}

	if (line_visibility < 0.5f)
	{
		line_visibility = line_visibility * 2.0f;
		line_visibility = 1.0f - ut::Pow(1.0f - line_visibility, 0.5f);
		line_visibility /= 2.0f;
	}
	else
	{
		line_visibility = (line_visibility - 0.5f) * 2.0f;
		line_visibility = 1.0f - ut::Pow(1.0f - line_visibility, 1.5f);
		line_visibility = 0.5f + line_visibility / 2.0f;
	}

	return parameters.line_min_visibility + line_visibility *
		(parameters.line_max_visibility - parameters.line_min_visibility);
}

// Calculates pixel offset for lines animation.
float StencilHighlight::CalculateLineOffset(const Parameters& parameters,
                                            float time_ms)
{
	return time_ms / parameters.line_movement_anim_speed_ms;
}

// Creates a pipeline state for the pass filling
// highlighted pixels with solid color.
PipelineState StencilHighlight::CreateFillPassPipelineState()
{
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, fill_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.compare_mask = static_cast<ut::uint32>(StencilReference::highlight);
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = static_cast<ut::uint32>(StencilReference::highlight);
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(info, clear_color_and_ds_pass).MoveOrThrow();
}

// Creates a pipeline state for the pass drawing
// animated lines over highlighted pixels.
PipelineState StencilHighlight::CreateLinePassPipelineState()
{
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, line_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.compare_mask = static_cast<ut::uint32>(StencilReference::highlight);
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = static_cast<ut::uint32>(StencilReference::highlight);
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(info, color_and_ds_pass).MoveOrThrow();
}

// Creates a pipeline state for the pass blending
// final highlight color with the original image.
PipelineState StencilHighlight::CreateBlendPassPipelineState()
{
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, blend_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::Operation::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::Operation::keep;
	info.depth_stencil_state.back.compare_mask = static_cast<ut::uint32>(StencilReference::highlight);
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = static_cast<ut::uint32>(StencilReference::highlight);
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(info, color_only_pass).MoveOrThrow();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//