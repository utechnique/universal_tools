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
// Performs different dithering effects.
class Dithering
{
public:
	class ViewData
	{
	public:
		ViewData(Buffer in_gradient_dithering_buffer);

		struct GradientDitheringDescriptorSet : public DescriptorSet
		{
			GradientDitheringDescriptorSet() : DescriptorSet(ub, tex2d,
			                                                 tex2d_lut,
			                                                 sampler,
			                                                 sampler_lut) {}

			Descriptor ub = "g_ub_gradient_dithering";
			Descriptor tex2d = "g_tex2d";
			Descriptor tex2d_lut = "g_tex2d_lut";
			Descriptor sampler = "g_sampler";
			Descriptor sampler_lut = "g_sampler_lut";
		} gradient_dithering_desc_set;

		struct GradientDitheringUB
		{
			alignas(16) ut::Vector<4> parameters;
		};

		Buffer gradient_dithering_buffer;
	};

	// Customisable effect parameters.
	struct Parameters
	{
		bool gradient_dithering_enabled = true;
	};

	// Constructor.
	Dithering(Toolset& toolset,
	          RenderPass& color_only_pass);

	// Creates dithering (per-view) data.
	//    @return - a new Dithering::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData();

	// Performs gradient dithering, makes smooth color transition for LDR targets.
	//    @param swap_mgr - reference to the post-process swap manager.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the Dithering::ViewData object.
	//    @param source - reference to the source image.
	// 	  @param parameters - reference to the Dithering::Parameters object
	//                        containing parameters for the dithering effect.
	//    @param time_ms - total accumulated time in milliseconds.
	//    @return - optional reference to the postprocess slot used for dithering.
	ut::Optional<SwapSlot&> ApplyGradientDithering(SwapManager& swap_mgr,
	                                               Context& context,
	                                               ViewData& data,
	                                               Image& source,
	                                               const Parameters& parameters,
	                                               float time_ms);

private:
	// Blue noise generation for the lookup table takes much time. It can be
	// cached to reduce dithering initialization time.
	struct Cache : public ut::meta::Reflective
	{
		Cache() : path(ut::String(directories::skCache) + ut::skFileSeparator + skFileName)
		{}

		// Name of the cache meta container.
		static const char* skMetaName;

		// Name of the cache file.
		static const char* skFileName;

		// The path to the cache file.
		ut::String path;

		// Cached data.
		ut::Array<ut::Color<2, ut::uint16>> lookup;

		// Registers @lookup data into reflection tree.
		//    @param snapshot - reference to the reflection tree
		void Reflect(ut::meta::Snapshot& snapshot);

		// Loads @lookup data from the @skCacheFileName file.
		ut::Optional<ut::Error> Load(size_t expected_lookup_size);

		// Saves @lookup data to the @skCacheFileName file.
		ut::Optional<ut::Error> Save();
	};

	// Returns compiled pixel shader performing the gradient dithering effect.
	Shader LoadGradientDitheringShader();

	// Creates a pipeline state for the gradient dithering effect.
	PipelineState CreateGradientDitheringPipelineState();

	// Generates dithering lookup table image.
	RcRef<Map> GenLookupTable();

	Toolset& tools;

	Mesh::Subset& fullscreen_quad;

	RenderPass& color_only_pass;

	Shader gradient_dithering_shader;

	PipelineState gradient_dithering_pipeline;

	RcRef<Map> lookup;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//