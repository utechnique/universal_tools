//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Interchangeable post-process target.
struct SwapSlot
{
	bool busy; // must be set to 'false' after using
	Target color_target;
	Framebuffer color_only_framebuffer;
	Framebuffer color_and_ds_framebuffer;
};

// Helps to swap post-process target buffers.
class SwapManager
{
public:
	static constexpr ut::uint32 skSlotCount = 3;

	// Constructor accepts an array of slots.
	SwapManager(ut::Array<SwapSlot> in_swap_slots);

	// Returns a reference to the next intermediate buffer.
	//    @return - optional reference to the post-process target. It has @busy
	//              member set to 'true' which must be changed to 'false' after
	//              using before the next Swap() call.
	ut::Optional<SwapSlot&> Swap();

private:
	ut::Array<SwapSlot> slots;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//