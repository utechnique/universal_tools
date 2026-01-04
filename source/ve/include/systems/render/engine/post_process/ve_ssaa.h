//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/post_process/ve_post_process_slots.h"
#include "ve_downsampling.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Supersampling anti-aliasing fights with jaggies by rendering the scene into
// high resolution supersampled buffer and then resolving it into a final
// downscaled image that can be displayed to the user.
class Ssaa
{
public:
	// This SSAA variant supports limited sample count values, only those
	// forming discrete pixel blocks that can be downsampled into 1 final
	// pixel. For example: 2x2, 3x3 and 4x4 pixel blocks.
	enum class SampleCount
	{
		s1, // no supersampling
		s4, // 2x2
		s9, // 3x3
		s16 // 4x4
	};

	// SSAA view data contains downsampling information essential to
	// resolve supersampled buffer.
	class ViewData
	{
	public:
		SampleCount sample_count;
		ut::Array<Downsampling::ViewData> downsampling_data;
	};

	// Constructor. Saves the references to therendering tools required
	// to resolve SSAA buffer.
	Ssaa(Toolset& toolset, Downsampling& downsampling);

	// Initializes downsampling data.
	//    @param width - original (non-supersampled) view width in pixels.
	//    @param width - original (non-supersampled) view height in pixels.
	//    @param sample_count - the number of SSAA samples.
	//    @return - Ssaa::ViewData object of ut::Error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(ut::uint32 width,
	                                               ut::uint32 height,
	                                               SampleCount sample_count);

	// Transforms provided supersampled buffer into the
	// final downsampled image.
	//    @param swap_mgr - a reference to the swap manager, it's needed to
	//                      provide a render target for the final downsampled
	//                      image and intermediate downsampling steps.
	//    @param context - a reference to the rendering context.
	//    @param data - a reference to the SSAA per-view data containing
	//                  downsampling information.
	//    @param source - a reference to the source supersampled buffer.
	//    @return - a reference to the swap slot containing the resolved image
	//              or nothing if SSAA is disabled (SampleCount::s1 value).
	ut::Optional<SwapSlot&> Resolve(SwapManager& swap_mgr,
	                                Context& context,
	                                ViewData& data,
	                                Image& source);

	// Converts provided size into the size of the supersampled buffer.
	static ut::Vector<2, ut::uint32> CalculateSupersampledSize(const ut::Vector<2, ut::uint32>& original_size,
	                                                           SampleCount sample_count);

private:
	Toolset& tools;
	Downsampling& downsampling;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//