//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Converts rgb image to srgb format.
class RgbToSrgb
{
public:
	class ViewData
	{
	public:
		ViewData(PipelineState converter_pipeline_state);

		struct RgbToSrgbDescriptorSet : public DescriptorSet
		{
			RgbToSrgbDescriptorSet() : DescriptorSet(tex2d, sampler) {}
			Descriptor tex2d = "g_tex2d";
			Descriptor sampler = "g_sampler";
		} desc_set;

		PipelineState pipeline_state;
	};

	// Constructor.
	RgbToSrgb(Toolset& toolset);

	// Creates srgb converter (per-view) data.
	//    @param postprocess_pass - render pass that will be used for fxaa.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @return - a new RgbToSrgb::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(RenderPass& postprocess_pass,
	                                               ut::uint32 width,
	                                               ut::uint32 height);

	// Performs rgb to srgb conversion.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the RgbToSrgb::ViewData object containing
	//                  converter-specific resources.
	//    @param fb - reference to the framebuffer (with bound destination target).
	//    @param pass - reference to the render pass with one color attachment
	//                  and no depth.
	//    @param source - reference to the source image.
	void Apply(Context& context,
	           ViewData& data,
	           Framebuffer& fb,
	           RenderPass& pass,
	           Image& source);

private:
	// Returns compiled rgb to srgb converter pixel shader.
	Shader LoadShader();

	// Common rendering tools.
	Toolset& tools;
	Shader pixel_shader;
};


//----------------------------------------------------------------------------//
// Tone mapping techniques.
class ToneMapper
{
public:
	class ViewData
	{
	public:
		ViewData(RgbToSrgb::ViewData rgb_to_srgb_data);

		RgbToSrgb::ViewData rgb_to_srgb;
	};

	// Constructor.
	ToneMapper(Toolset& toolset);

	// Creates tone mapping (per-view) data.
	//    @param postprocess_pass - render pass that will be used for fxaa.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @return - a new ToneMapper::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(RenderPass& postprocess_pass,
	                                               ut::uint32 width,
	                                               ut::uint32 height);

	// Performs tone mapping.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the ToneMapper::ViewData object containing
	//                  tonemap-specific resources.
	//    @param fb - reference to the framebuffer (with bound destination target).
	//    @param pass - reference to the render pass with one color attachment
	//                  and no depth.
	//    @param source - reference to the source image.
	void Apply(Context& context,
	           ViewData& data,
	           Framebuffer& fb,
	           RenderPass& pass,
	           Image& source);

private:
	// Common rendering tools.
	Toolset& tools;

	// Converts rgb images to srgb.
	RgbToSrgb rgb_to_srgb;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
