//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/post_process/ve_post_process_slots.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Just clamps the hdr image to fit ldr.
class ClampToneMapper
{
public:
	class ViewData
	{
	public:
		ViewData(PipelineState converter_pipeline_state);

		struct ClampToneMapperDescriptorSet : public DescriptorSet
		{
			ClampToneMapperDescriptorSet() : DescriptorSet(tex2d, sampler) {}
			Descriptor tex2d = "g_tex2d";
			Descriptor sampler = "g_sampler";
		} desc_set;

		PipelineState pipeline_state;
	};

	// Customisable parameters.
	struct Parameters
	{
		bool enabled = true;
	};

	// Constructor.
	ClampToneMapper(Toolset& toolset);

	// Creates per-view data.
	//    @param postprocess_pass - render pass that will be used for mapping.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @return - a new ClampToneMapper::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(RenderPass& postprocess_pass,
	                                               ut::uint32 width,
	                                               ut::uint32 height);

	// Performs hdr to ldr conversion.
	//    @param swap_mgr - reference to the post-process swap manager.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the ClampToneMapper::ViewData object containing
	//                  converter-specific resources.
	//    @param pass - reference to the render pass with one color attachment
	//                  and no depth.
	//    @param source - reference to the source image.
	//    @return - reference to the postprocess slot used for tone mapping.
	SwapSlot& Apply(SwapManager& swap_mgr,
	                Context& context,
	                ViewData& data,
	                RenderPass& pass,
	                Image& source);

private:
	// Returns compiled mapping pixel shader.
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
		ViewData(ClampToneMapper::ViewData clamp_mapper_data);

		ClampToneMapper::ViewData clamp_mapper_data;
	};

	// Customisable parameters.
	struct Parameters
	{
		bool enabled = true;
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
	ut::Optional<SwapSlot&> Apply(SwapManager& swap_mgr,
	                              Context& context,
	                              ViewData& data,
	                              RenderPass& pass,
	                              Image& source,
	                              const Parameters& parameters);

private:
	// Common rendering tools.
	Toolset& tools;

	// A tone-mapper to be used for hdr->ldr convertion.
	ClampToneMapper clamp_mapper;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
