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
// Stencil highlighting (per-view) data constructor.
StencilHighlight::ViewData::ViewData(PipelineState in_fill_pass_pipeline,
                                     PipelineState in_lines_pass_pipeline,
                                     PipelineState in_blend_pass_pipeline,
                                     Buffer in_white_color_buffer,
                                     Buffer in_lines_color_buffer,
                                     Buffer in_blend_color_buffer,
                                     GaussianBlur::ViewData horizontal_blur_data,
                                     GaussianBlur::ViewData vertical_blur_data) : fill_pass_pipeline(ut::Move(in_fill_pass_pipeline))
                                                                                , lines_pass_pipeline(ut::Move(in_lines_pass_pipeline))
                                                                                , blend_pass_pipeline(ut::Move(in_blend_pass_pipeline))
                                                                                , white_color_buffer(ut::Move(in_white_color_buffer))
                                                                                , lines_color_buffer(ut::Move(in_lines_color_buffer))
                                                                                , blend_color_buffer(ut::Move(in_blend_color_buffer))
                                                                                , horizontal_blur(ut::Move(horizontal_blur_data))
                                                                                , vertical_blur(ut::Move(vertical_blur_data))
{}

// Stencil highlighting constructor.
StencilHighlight::StencilHighlight(Toolset& toolset,
                                   GaussianBlur& gaussian_blur) : tools(toolset)
                                                                , blur(gaussian_blur)
                                                                , fill_shader(LoadFillShader())
                                                                , line_shader(LoadLineShader())
                                                                , blend_shader(LoadBlendShader())
                                                                , blur_shader(gaussian_blur.LoadShader(skHighlightRadius,
                                                                                                       skHighlightSigma).MoveOrThrow())
{}

// Returns compiled pixel shader filling a surface with solid color.
Shader StencilHighlight::LoadFillShader()
{
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::pixel,
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

	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::pixel,
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
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::pixel,
	                                                                "stencil_highlight_blend_ps",
	                                                                "BlendPS",
	                                                                "highlight.hlsl");
	return shader.MoveOrThrow();
}

// Creates highlighting (per-view) data.
//    @param color_only_pass - reference to the render pass with one
//                             color attachment and no depth.
//    @param color_and_ds_pass - reference to the render pass with one
//                               color and one depth-stencil attachment.
//    @param clear_color_and_ds_pass - reference to the render pass with one
//                                     color and one depth-stencil attachment,
//                                     color attachment is set to be cleared.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @return - a new StencilHighlight::ViewData object or error if failed.
ut::Result<StencilHighlight::ViewData, ut::Error> StencilHighlight::CreateViewData(RenderPass& color_only_pass,
                                                                                   RenderPass& color_and_ds_pass,
                                                                                   RenderPass& clear_color_and_ds_pass,
                                                                                   ut::uint32 width,
                                                                                   ut::uint32 height)
{
	// create pipelines
	PipelineState::Info info;
	info.stages[Shader::vertex] = tools.shaders.quad_vs;
	info.stages[Shader::pixel] = fill_shader;
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            width, height));
	info.input_assembly_state = tools.rc_mgr.fullscreen_quad->input_assembly;
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::never;
	info.depth_stencil_state.stencil_test_enable = true;
	info.depth_stencil_state.back.compare_op = compare::equal;
	info.depth_stencil_state.back.fail_op = StencilOpState::keep;
	info.depth_stencil_state.back.pass_op = StencilOpState::keep;
	info.depth_stencil_state.back.compare_mask = stencilref_highlight;
	info.depth_stencil_state.front = info.depth_stencil_state.back;
	info.depth_stencil_state.stencil_write_mask = 0x0;
	info.depth_stencil_state.stencil_reference = stencilref_highlight;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	ut::Result<PipelineState, ut::Error> fill_pipeline = tools.device.CreatePipelineState(info, clear_color_and_ds_pass);
	if (!fill_pipeline)
	{
		return ut::MakeError(fill_pipeline.MoveAlt());
	}
	info.stages[Shader::pixel] = line_shader;
	ut::Result<PipelineState, ut::Error> lines_pipeline = tools.device.CreatePipelineState(info, color_and_ds_pass);
	if (!lines_pipeline)
	{
		return ut::MakeError(lines_pipeline.MoveAlt());
	}
	info.stages[Shader::pixel] = blend_shader;
	ut::Result<PipelineState, ut::Error> blend_pipeline = tools.device.CreatePipelineState(info, color_only_pass);
	if (!blend_pipeline)
	{
		return ut::MakeError(blend_pipeline.MoveAlt());
	}

	// create uniform buffers
	ut::Vector<4> white(1.0f);
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::uniform;
	buffer_info.usage = render::memory::gpu_immutable;
	buffer_info.size = sizeof(ut::Vector<4>);
	buffer_info.data.Resize(buffer_info.size);
	ut::memory::Copy(buffer_info.data.GetAddress(), white.GetData(), buffer_info.size);
	ut::Result<Buffer, ut::Error> white_color_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!white_color_buffer)
	{
		throw ut::Error(white_color_buffer.MoveAlt());
	}
	buffer_info.usage = render::memory::gpu_read_cpu_write;
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
	                                                                                    width, height,
	                                                                                    skHighlightRadius,
	                                                                                    skHighlightSigma);
	if (!horizontal_blur)
	{
		return ut::MakeError(horizontal_blur.MoveAlt());
	}

	// create vertical gaussian blur data
	ut::Result<GaussianBlur::ViewData, ut::Error> vertical_blur = blur.CreateViewData(color_only_pass,
	                                                                                  blur_shader,
	                                                                                  width, height,
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
	StencilHighlight::ViewData data(fill_pipeline.Move(),
	                                lines_pipeline.Move(),
	                                blend_pipeline.Move(),
	                                white_color_buffer.Move(),
	                                lines_color_buffer.Move(),
	                                blend_color_buffer.Move(),
	                                horizontal_blur.Move(),
	                                vertical_blur.Move());

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
//    @param color_only_pass - reference to the render pass with one
//                             color attachment and no depth.
//    @param color_and_ds_pass - reference to the render pass with one
//                               color and one depth-stencil attachment.
//    @param clear_color_and_ds_pass - reference to the render pass with one
//                                     color and one depth-stencil attachment,
//                                     color attachment is set to be cleared.
//    @param source - reference to the source image.
//    @param time_ms - total accumulated time in milliseconds.
//    @param highlight_color - border color of the highlighted objects.
//    @return - reference to the postprocess slot used for highlighting.
SwapSlot& StencilHighlight::Apply(SwapManager& swap_mgr,
                                  Context& context,
                                  ViewData& data,
                                  RenderPass& color_only_pass,
                                  RenderPass& color_and_ds_pass,
                                  RenderPass& clear_color_and_ds_pass,
                                  Image& source,
                                  float time_ms,
                                  const ut::Vector<4>& highlight_color)
{
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
	context.BindPipelineState(data.fill_pass_pipeline);
	context.BindDescriptorSet(data.fill_desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->vertex_buffer, 0);
	context.Draw(6, 0);
	context.EndRenderPass();
	context.SetTargetState(fill_slot->color_target, Target::Info::state_resource);

	// blur image to extend object borders
	ut::Optional<SwapSlot&> hblur_slot = swap_mgr.Swap();
	UT_ASSERT(hblur_slot.HasValue());
	blur.Apply(context,
	           data.horizontal_blur,
	           hblur_slot->color_only_framebuffer,
	           color_only_pass,
	           fill_slot->color_target.GetImage(),
	           GaussianBlur::direction_horizontal);
	context.SetTargetState(hblur_slot->color_target, Target::Info::state_resource);
	fill_slot->busy = false;
	ut::Optional<SwapSlot&> vblur_slot = swap_mgr.Swap();
	UT_ASSERT(hblur_slot.HasValue());
	blur.Apply(context,
	           data.vertical_blur,
	           vblur_slot->color_only_framebuffer,
	           color_only_pass,
	           hblur_slot->color_target.GetImage(),
	           GaussianBlur::direction_vertical);
	hblur_slot->busy = false;

	// update line buffer
	ut::Vector<4> lines_data(fb_info.width,
	                         fb_info.height,
	                         CalculateLineVisibility(time_ms),
	                         CalculateLineOffset(time_ms));
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
	context.BindPipelineState(data.lines_pass_pipeline);
	context.BindDescriptorSet(data.lines_desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->vertex_buffer, 0);
	context.Draw(6, 0);
	context.EndRenderPass();
	context.SetTargetState(vblur_slot->color_target, Target::Info::state_resource);

	// update blend color buffer
	update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                            data.blend_color_buffer,
	                                            highlight_color.GetData());
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
	context.BindPipelineState(data.blend_pass_pipeline);
	context.BindDescriptorSet(data.blend_desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->vertex_buffer, 0);
	context.Draw(6, 0);
	context.EndRenderPass();
	vblur_slot->busy = false;

	return blend_slot.Get();
}

// Calculates visibility factor for lines animation.
float StencilHighlight::CalculateLineVisibility(float time_ms)
{
	const int visibility_period = static_cast<int>(time_ms / skLineVisibilityAnimationSpeedMs);
	float line_visibility = time_ms - static_cast<float>(visibility_period) *
	                        skLineVisibilityAnimationSpeedMs;
	line_visibility /= skLineVisibilityAnimationSpeedMs;

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
		line_visibility = 1.0f - ut::Pow(1.0f - line_visibility, 1.4f);
		line_visibility /= 2.0f;
	}
	else
	{
		line_visibility = (line_visibility - 0.5f) * 2.0f;
		line_visibility = 1.0f - ut::Pow(1.0f - line_visibility, 1.4f);
		line_visibility = 0.5f + line_visibility / 2.0f;
	}

	return skLineMinVisibility + line_visibility * (skLineMaxVisibility - skLineMinVisibility);
}

// Calculates pixel offset for lines animation.
float StencilHighlight::CalculateLineOffset(float time_ms)
{
	return time_ms / skLineMovementAnimationSpeedMs;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//