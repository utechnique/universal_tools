//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_gaussian_blur.h"
#include "systems/render/engine/post_process/ve_post_process_slots.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Highlights an area of viewport marked with stencilref_highlight stencil bit.
class StencilHighlight
{
	static constexpr ut::uint32 skHighlightRadius = 3;
	static constexpr float skHighlightSigma = skHighlightRadius / 0.2f;
	static constexpr float skLineMovementAnimationSpeedMs = 55.0f;
	static constexpr float skLineVisibilityAnimationSpeedMs = 2000.0f;
	static constexpr float skLineMinVisibility = 0.075f;
	static constexpr float skLineMaxVisibility = 0.5f;
	static constexpr ut::uint32 skLineDistance = 16;
	static constexpr ut::uint32 skLineWidth = 8;

public:
	class ViewData
	{
	public:
		ViewData(PipelineState highlight_fill_pass_pipeline,
		         PipelineState highlight_lines_pass_pipeline,
		         PipelineState highlight_blend_pass_pipeline,
		         Buffer white_color_buffer,
		         Buffer lines_color_buffer,
		         Buffer blend_color_buffer,
		         GaussianBlur::ViewData horizontal_blur_data,
		         GaussianBlur::ViewData vertical_blur_data);

		struct FillDescriptorSet : public DescriptorSet
		{
			FillDescriptorSet() : DescriptorSet(ub_color) {}
			Descriptor ub_color = "g_ub_highlight_color";
		} fill_desc_set;

		struct DrawLinesDescriptorSet : public DescriptorSet
		{
			DrawLinesDescriptorSet() : DescriptorSet(ub_color) {}
			Descriptor ub_color = "g_ub_highlight_color";
		} lines_desc_set;

		struct BlendDescriptorSet : public DescriptorSet
		{
			BlendDescriptorSet() : DescriptorSet(ub_color, input_tex2d,
			                                     highlight_tex2d, sampler) {}
			Descriptor ub_color = "g_ub_highlight_color";
			Descriptor input_tex2d = "g_input_texture";
			Descriptor highlight_tex2d = "g_highlight_texture";
			Descriptor sampler = "g_sampler";
		} blend_desc_set;

		PipelineState fill_pass_pipeline;
		PipelineState lines_pass_pipeline;
		PipelineState blend_pass_pipeline;
		Buffer white_color_buffer;
		Buffer lines_color_buffer;
		Buffer blend_color_buffer;
		GaussianBlur::ViewData horizontal_blur;
		GaussianBlur::ViewData vertical_blur;
	};

	// Constructor.
	StencilHighlight(Toolset& toolset,
	                 GaussianBlur& gaussian_blur);

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
	ut::Result<ViewData, ut::Error> CreateViewData(RenderPass& color_only_pass,
	                                               RenderPass& color_and_ds_pass,
	                                               RenderPass& clear_color_and_ds_pass,
	                                               ut::uint32 width,
	                                               ut::uint32 height);

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
	SwapSlot& Apply(SwapManager& swap_mgr,
	                Context& context,
	                ViewData& data,
	                RenderPass& color_only_pass,
	                RenderPass& color_and_ds_pass,
	                RenderPass& clear_color_and_ds_pass,
	                Image& source,
	                float time_ms,
	                const ut::Vector<4>& highlight_color = ut::Vector<4>(1.0f, 0.75f, 0.0f, 1.0f));

private:
	// Returns compiled pixel shader filling a surface with solid color.
	Shader LoadFillShader();

	// Returns compiled pixel shader drawing diagonal lines.
	Shader LoadLineShader();

	// Returns compiled pixel shader blending source texture with
	// highlighting mask.
	Shader LoadBlendShader();

	// Calculates visibility factor for lines animation.
	static float CalculateLineVisibility(float time_ms);

	// Calculates pixel offset for lines animation.
	static float CalculateLineOffset(float time_ms);

	// Common rendering tools.
	Toolset& tools;
	GaussianBlur& blur;
	Shader fill_shader;
	Shader line_shader;
	Shader blend_shader;
	Shader blur_shader;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
