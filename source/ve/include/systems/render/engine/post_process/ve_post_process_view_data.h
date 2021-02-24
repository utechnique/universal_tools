//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_tone_mapping.h"
#include "ve_fxaa.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// ve::render::postprocess::ViewData encapsulates render targets, uniform
// buffers, pipelines and other resources needed to apply post-process effects
// on a ve::render::View object.
class ViewData
{
public:
	static constexpr ut::uint32 skSwapSlotCount = 2;

	// Intermediate buffer.
	struct SwapSlot
	{
		Target target;
		Framebuffer framebuffer;
	};

	// Constructor.
	ViewData(ut::Array<SwapSlot> in_swap_slots,
	         RenderPass in_pass,
	         ToneMapper::ViewData in_tone_mapping,
	         Fxaa::ViewData in_fxaa);

	// Move constructor and operator.
	ViewData(ViewData&&) = default;
	ViewData& operator =(ViewData&&) = default;

	// Copying is prohibited.
	ViewData(const ViewData&) = delete;
	ViewData& operator =(const ViewData&) = delete;

	// Returns a reference to the next intermediate buffer.
	SwapSlot& Swap();

	// intermediate buffers
	ut::Array<SwapSlot> swap_slots;
	ut::uint32 swap_slot_id = 0;

	// render pass for all effects
	RenderPass pass;

	// effects
	ToneMapper::ViewData tone_mapping;
	Fxaa::ViewData fxaa;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
