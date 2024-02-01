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
// FXAA effect.
class Fxaa
{
public:
	class ViewData
	{
	public:
		// Constructor.
		ViewData(PipelineState fxaa_pipeline_state,
		         Buffer fxaa_uniform_buffer);

		// Move constructor and operator.
		ViewData(ViewData&&) noexcept = default;
		ViewData& operator =(ViewData&&) noexcept = default;

		// Copying is prohibited.
		ViewData(const ViewData&) = delete;
		ViewData& operator =(const ViewData&) = delete;

		struct FxaaDescriptorSet : public DescriptorSet
		{
			FxaaDescriptorSet() : DescriptorSet(ub, tex2d, sampler) {}
			Descriptor ub = "g_ub_fxaa";
			Descriptor tex2d = "g_tex2d";
			Descriptor sampler = "g_sampler";
		} desc_set;

		struct FxaaUB
		{
			alignas(16) ut::Vector<4> texel_size;
		};

		PipelineState pipeline_state;
		Buffer uniform_buffer;
	};

	// Constructor.
	Fxaa(Toolset& toolset);

	// Creates fxaa (per-view) data.
	//    @param postprocess_pass - render pass that will be used for fxaa.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @return - a new Fxaa::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(RenderPass& postprocess_pass,
	                                               ut::uint32 width,
	                                               ut::uint32 height);

	// Applies fxaa effect.
	//    @param swap_mgr - reference to the post-process swap manager.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the Fxaa::ViewData object containing
	//                  fxaa-specific resources.
	//    @param pass - reference to the render pass with one color attachment
	//                  and no depth.
	//    @param source - reference to the source image.
	//    @return - reference to the postprocess slot used for FXAA.
	SwapSlot& Apply(SwapManager& swap_mgr,
	                Context& context,
	                ViewData& data,
	                RenderPass& pass,
	                Image& source);

private:
	// Returns compiled fxaa pixel shader.
	Shader LoadFxaaShader();

	// Generates an array of macros for fxaa shader.
	//    @param preset_id - quality preset (from 0 to 5).
	//    @return - an array of macros.
	static Shader::Macros GenShaderMacros(ut::uint32 preset_id);

	// FXAA_PRESET shader macro value.
	static const ut::uint32 skDefaultPresetId;

	// Common rendering tools.
	Toolset& tools;

	// FXAA pixel shader.
	Shader pixel_shader;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
