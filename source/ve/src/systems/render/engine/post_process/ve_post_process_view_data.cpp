//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_post_process_view_data.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Constructor.
ViewData::ViewData(ut::Array<SwapSlot> in_swap_slots,
                   RenderPass in_pass,
                   ToneMapper::ViewData in_tone_mapping,
                   Fxaa::ViewData in_fxaa) : swap_slots(ut::Move(in_swap_slots))
                                           , pass(ut::Move(in_pass))
                                           , tone_mapping(ut::Move(in_tone_mapping))
                                           , fxaa(ut::Move(in_fxaa))
{
	UT_ASSERT(swap_slots.GetNum() == skSwapSlotCount);
}

// Returns a reference to the next intermediate buffer.
ViewData::SwapSlot& ViewData::Swap()
{
	ViewData::SwapSlot& out = swap_slots[swap_slot_id];
	if (++swap_slot_id >= skSwapSlotCount)
	{
		swap_slot_id = 0;
	}
	return out;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//