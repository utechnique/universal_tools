//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::RenderTargetSlot represents a slot in a renderpass for the
// render target. So that only a special framebuffer where every target matches
// corresponding slot can be bound to it.
class RenderTargetSlot
{
public:
	enum LoadOperation
	{
		// extract previous value from memory
		load_extract,

		// all contents will be cleared
		load_clear,

		// nothing to do
		load_dont_care
	};

	enum StoreOperation
	{
		// final value will be stored in memory
		store_save,

		// nothing to do
		store_dont_care
	};

	// Constructor
	RenderTargetSlot(pixel::Format in_format,
	                 LoadOperation in_load_op,
	                 StoreOperation in_store_op,
	                 bool is_present_surface) : format(in_format)
	                                          , load_op(in_load_op)
	                                          , store_op(in_store_op)
	                                          , present_surface(is_present_surface)
	{}

	// pixel format of the render target
	pixel::Format format;

	// specifies how contents of a render target are treated
	// at the beginning of a render pass
	LoadOperation load_op;

	// specifies how contents of a render target are treated
	// at the end of a render pass
	StoreOperation store_op;

	// specifies that slot is intended to be used by one of the buffers
	// in a swap chain to display final image to user
	bool present_surface;
};

// ve::render::RenderPass  represents a collection of slots for render target
// and describes how targets are used.
class RenderPass : public PlatformRenderPass
{
	friend class Context;
	friend class Device;

public:
	// Constructor.
	RenderPass(PlatformRenderPass platform_render_pass,
	           ut::Array<RenderTargetSlot> in_color_slots,
	           ut::Optional<RenderTargetSlot> in_depth_stencil_slot);

	// Move constructor.
	RenderPass(RenderPass&&) noexcept;

	// Move operator.
	RenderPass& operator =(RenderPass&&) noexcept;

	// Copying is prohibited.
	RenderPass(const RenderPass&) = delete;
	RenderPass& operator =(const RenderPass&) = delete;

private:
	ut::Array<RenderTargetSlot> color_slots;
	ut::Optional<RenderTargetSlot> depth_stencil_slot;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//