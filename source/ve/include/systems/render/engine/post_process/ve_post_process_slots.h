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
typedef ut::Array<SwapSlot> SwapSlots;

// Helps to swap post-process target buffers.
class SwapManager
{
public:
	// The postprocess chain is divided into 2 separate stages.
	// The first one takes place before SSAA resolving and operates with
	// supersampled buffers, and the second one applies effects after SSAA
	// resolving and operates with downscaled buffers.
	enum class Stage
	{
		supersampled,
		resolved,
		count
	};

	// The number of postprocess stages.
	static constexpr ut::uint32 skStageCount = static_cast<ut::uint32>(Stage::count);

	// The number of swap buffers for each stage.
	static constexpr ut::uint32 skSlotCount = 3;

	// Constructor accepts an array of slots.
	SwapManager(ut::Array<SwapSlots> in_slots);

	// Assigns a new stage that will be used for Swap() function calls.
	void SetStage(Stage new_stage);

	// Returns a reference to the next intermediate buffer.
	//    @return - optional reference to the post-process target. It has @busy
	//              member set to 'true' which must be changed to 'false' after
	//              using before the next Swap() call.
	ut::Optional<SwapSlot&> Swap();

private:
	Stage stage = Stage::supersampled;
	ut::Array<SwapSlots> slots;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//