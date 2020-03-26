//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_display.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Display::Display(PlatformDisplay platform_display,
                 Target display_target,
                 ut::uint32 w, ut::uint32 h) : PlatformDisplay(ut::Move(platform_display))
                                             , target(ut::Move(display_target))
                                             , width(w)
                                             , height(h)
{}

// Move constructor.
Display::Display(Display&&) noexcept = default;

// Move operator.
Display& Display::operator =(Display&&) noexcept = default;

// Returns width of the display in pixels.
ut::uint32 Display::GetWidth() const
{
	return width;
}

// Returns height of the display in pixels.
ut::uint32 Display::GetHeight() const
{
	return height;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//