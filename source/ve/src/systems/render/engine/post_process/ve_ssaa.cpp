//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_ssaa.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Constructor. Saves the references to therendering tools required
// to resolve SSAA buffer.
Ssaa::Ssaa(Toolset& in_toolset,
           Downsampling& in_downsampling) : tools(in_toolset)
                                          , downsampling(in_downsampling)
{}

// Initializes downsampling data.
//    @param width - original (non-supersampled) view width in pixels.
//    @param width - original (non-supersampled) view height in pixels.
//    @param sample_count - the number of SSAA samples.
//    @return - Ssaa::ViewData object of ut::Error if failed.
ut::Result<Ssaa::ViewData, ut::Error> Ssaa::CreateViewData(ut::uint32 width,
                                                           ut::uint32 height,
                                                           SampleCount sample_count)
{
	// initialize downsampling data
	ut::Array<Downsampling::ViewData> downsampling_data;
	ut::Vector<2, ut::uint32> supersampled_size = CalculateSupersampledSize(ut::Vector<2, ut::uint32>(width, height),
	                                                                        sample_count);
	
	const Downsampling::Granularity granularity = sample_count == SampleCount::s9 ?
	                                              Downsampling::Granularity::s3x3 :
	                                              Downsampling::Granularity::s2x2;
	const Downsampling::Filter filter = Downsampling::Filter::bilinear;
	ut::Result<Downsampling::ViewData,
	           ut::Error> downsampling_step = downsampling.CreateViewData(supersampled_size.X(),
	                                                                      supersampled_size.Y(),
	                                                                      granularity,
	                                                                      filter);
	if (!downsampling_step)
	{
		return ut::MakeError(downsampling_step.MoveAlt());
	}

	downsampling_data.Add(downsampling_step.Move());

	// 4x4 sample variant requires additional downsampling step
	if (sample_count == SampleCount::s16)
	{
		supersampled_size = Downsampling::CalculateDownscaledSize(supersampled_size,
		                                                          granularity);

		downsampling_step = downsampling.CreateViewData(supersampled_size.X(),
	                                                    supersampled_size.Y(),
	                                                    granularity,
		                                                filter);
		if (!downsampling_step)
		{
			return ut::MakeError(downsampling_step.MoveAlt());
		}

		downsampling_data.Add(downsampling_step.Move());
	}

	// create final data object
	ViewData data =
	{
		sample_count,
		ut::Move(downsampling_data)
	};

	// success
	return data;
}

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
ut::Optional<SwapSlot&> Ssaa::Resolve(SwapManager& swap_mgr,
                                      Context& context,
                                      ViewData& data,
                                      Image& source)
{
	// exit if supersampling is disabled
	if (data.sample_count == SampleCount::s1)
	{
		return ut::Optional<SwapSlot&>();
	}

	// process downsampling steps, one by one
	ut::Optional<SwapSlot&> prev_slot, slot;
	const ut::uint32 downsampling_iteration_count =
		static_cast<ut::uint32>(data.downsampling_data.Count());
	for (ut::uint32 i = 0; i < downsampling_iteration_count; i++)
	{
		const bool is_first_step = i == 0;
		const bool is_last_step = i == downsampling_iteration_count - 1;

		Image& step_src = is_first_step ? source : prev_slot->color_target.GetImage();

		// all postprocess effects must use downsampled slots after the resolve
		if (is_last_step)
		{
			swap_mgr.SetStage(SwapManager::Stage::resolved);
		}

		// get a new slot to be used as a render target for downsampling
		slot = swap_mgr.Swap();

		// intermadiate downsampling steps occuring in the middle of the
		// downsampling chain are rendered to a slot of supersampled resolution,
		// but use only a part of it's render target, thus texture coordinates
		// must be modificated in order to be correctly sampled in the next steps
		Downsampling::ViewData& downsampling_step_data = data.downsampling_data[i];
		ut::Vector<2, float> texcoord_factor(1.0f);
		if (!is_first_step)
		{
			texcoord_factor.X() = downsampling_step_data.downsampled_size.X() /
			                      downsampling_step_data.original_size.X();
			texcoord_factor.Y() = downsampling_step_data.downsampled_size.Y() /
			                      downsampling_step_data.original_size.Y();
		}

		// perform downsampling
		downsampling.Apply(downsampling_step_data,
		                   context,
		                   slot->color_only_framebuffer,
		                   step_src,
		                   texcoord_factor);

		// release the previous slot
		if (prev_slot)
		{
			prev_slot->busy = false;
		}

		// perform a render target state transition and save current
		// slot that will be used as a resource in the next steps
		if (slot)
		{
			context.SetTargetState(slot->color_target, Target::Info::State::resource);
			prev_slot = slot;
		}
	}

	return slot;
}

// Converts provided @original_size into the size of the supersampled buffer.
ut::Vector<2, ut::uint32> Ssaa::CalculateSupersampledSize(const ut::Vector<2, ut::uint32>& original_size,
                                                          SampleCount sample_count)
{
	switch (sample_count)
	{
	case SampleCount::s1: return original_size;
	case SampleCount::s4: return original_size.ElementWise() * 2;
	case SampleCount::s9: return original_size.ElementWise() * 3;
	case SampleCount::s16: return original_size.ElementWise() * 4;
	default: return original_size;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//