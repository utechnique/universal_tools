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
                 ut::Array<Target> display_targets,
                 ut::uint32 w, ut::uint32 h) : PlatformDisplay(ut::Move(platform_display))
                                             , targets(ut::Move(display_targets))
                                             , width(w)
                                             , height(h)
                                             , current_buffer_id(0)
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

// Returns target representing a view of one of the buffers
// in a swapchain.
//    @param buffer_id - id of the swapchain buffer.
//    @return - reference to the render target.
Target& Display::GetTarget(ut::uint32 buffer_id)
{
	return targets[buffer_id];
}

// Returns the id of the buffer that is ready to be
// filled in the current frame.
ut::uint32 Display::GetCurrentBufferId() const
{
	return current_buffer_id;
}

// Returns a number of buffers in the associated swapchain.
ut::uint32 Display::GetBufferCount() const
{
	return static_cast<ut::uint32>(targets.GetNum());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//