//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_post_process_slots.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Constructor.
SwapManager::SwapManager(ut::Array<SwapSlots> in_slots) : slots(ut::Move(in_slots))
{
	UT_ASSERT(slots.Count() == skStageCount);
	for (const SwapSlots& stage_slots : slots)
	{
		UT_ASSERT(stage_slots.Count() == skSlotCount);
	}
}

// Assigns a new stage that will be used for Swap() function calls.
void SwapManager::SetStage(Stage new_stage)
{
	stage = new_stage;
}

// Returns a reference to the next intermediate buffer.
//    @return - optional reference to the post-process target. It has @busy
//              member set to 'true' which must be changed to 'false' after
//              using before the next Swap() call.
ut::Optional<SwapSlot&> SwapManager::Swap()
{
	UT_ASSERT(slots.Count() == skStageCount);
	SwapSlots& stage_slots = slots[static_cast<ut::uint32>(stage)];

	UT_ASSERT(stage_slots.Count() == skSlotCount);
	for (ut::uint32 i = 0; i < skSlotCount; i++)
	{
		SwapSlot& slot = stage_slots[i];
		if (!slot.busy)
		{
			slot.busy = true;
			return slot;
		}
	}

	return ut::Optional<SwapSlot&>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//