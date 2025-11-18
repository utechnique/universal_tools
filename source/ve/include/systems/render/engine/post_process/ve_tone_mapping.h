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
		struct ClampToneMapperDescriptorSet : public DescriptorSet
		{
			ClampToneMapperDescriptorSet() : DescriptorSet(tex2d, sampler) {}
			Descriptor tex2d = "g_tex2d";
			Descriptor sampler = "g_sampler";
		} desc_set;
	};

	// Customisable parameters.
	struct Parameters
	{
		bool enabled = true;
	};

	// Constructor.
	ClampToneMapper(Toolset& toolset, RenderPass& postprocess_pass);

	// Creates per-view data.
	//    @return - a new ClampToneMapper::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData();

	// Performs hdr to ldr conversion.
	//    @param swap_mgr - reference to the post-process swap manager.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the ClampToneMapper::ViewData object containing
	//                  converter-specific resources.
	//    @param source - reference to the source image.
	//    @return - reference to the postprocess slot used for tone mapping.
	SwapSlot& Apply(SwapManager& swap_mgr,
	                Context& context,
	                ViewData& data,
	                Image& source);

private:
	// Returns compiled mapping pixel shader.
	Shader LoadShader();

	// Creates a pipeline state for this tonemapper effect.
	PipelineState CreatePipelineState();

	// Common rendering tools.
	Toolset& tools;
	Mesh::Subset& fullscreen_quad;
	RenderPass& pass;
	Shader pixel_shader;
	PipelineState pipeline_state;
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
	ToneMapper(Toolset& toolset, RenderPass& postprocess_pass);

	// Creates tone mapping (per-view) data.
	//    @return - a new ToneMapper::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData();

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
	ut::Optional<SwapSlot&> Apply(SwapManager& swap_mgr,
	                              Context& context,
	                              ViewData& data,
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
