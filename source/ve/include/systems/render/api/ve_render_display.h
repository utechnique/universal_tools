//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_target.h"
#include "systems/render/api/ve_render_cmd_buffer.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Display is a rendering area inside a UI viewport (widget/window).
class Display : public PlatformDisplay
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	Display(PlatformDisplay platform_display,
	        ut::Array<Target> display_targets,
	        ut::uint32 display_width,
	        ut::uint32 display_height,
	        bool vertical_sync);

	// Move constructor.
	Display(Display&&) noexcept;

	// Move operator.
	Display& operator =(Display&&) noexcept;

	// Copying is prohibited.
	Display(const Display&) = delete;
	Display& operator =(const Display&) = delete;

	// Returns width of the display in pixels.
	ut::uint32 GetWidth() const;

	// Returns height of the display in pixels.
	ut::uint32 GetHeight() const;

	// Returns target representing a view of one of the buffers
	// in a swapchain.
	//    @param buffer_id - id of the swapchain buffer.
	//    @return - reference to the render target.
	Target& GetTarget(ut::uint32 buffer_id);

	// Returns the id of the buffer that is ready to be
	// filled in the current frame.
	ut::uint32 GetCurrentBufferId() const;

	// Returns a number of buffers in the associated swapchain.
	ut::uint32 GetBufferCount() const;
private:
	// Render targets associated with buffers of this display.
	ut::Array<Target> targets;

	// Id of the buffer that is ready to be filled in the current frame.
	ut::uint32 current_buffer_id;

	// Width of the display in pixels.
	ut::uint32 width;

	// Height of the display in pixels.
	ut::uint32 height;

	// Whether vertical synchronization is enabled for this display.
	bool vsync;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//