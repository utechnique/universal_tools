//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_target.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Display is a rendering area inside a UI viewport (widget/window).
class Display : public PlatformDisplay
{
	friend class Context;
	friend class Device;
public:
	// Constructor.
	Display(PlatformDisplay platform_display,
	        Target display_target,
	        ut::uint32 w, ut::uint32 h);

	// Move constructor.
	Display(Display&&) noexcept;

	// Move operator.
	Display& operator =(Display&&) noexcept;

	// Copying is prohibited.
	Display(const Display&) = delete;
	Display& operator =(const Display&) = delete;

	// Presents a rendered image to the user.
	//    @param vsync - 'true' to enable vertical synchronization.
	void Present(bool vsync);

	// Returns width of the display in pixels.
	ut::uint32 GetWidth() const;

	// Returns height of the display in pixels.
	ut::uint32 GetHeight() const;

	// Render target associated with this display.
	Target target;

private:
	// Width of the display in pixels.
	ut::uint32 width;

	// Height of the display in pixels.
	ut::uint32 height;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//