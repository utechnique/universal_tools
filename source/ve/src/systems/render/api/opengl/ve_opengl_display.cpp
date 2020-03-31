//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_display.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformDisplay::PlatformDisplay(Fl_Gl_Window* opengl_window) : window(opengl_window)
{}

// Move constructor.
PlatformDisplay::PlatformDisplay(PlatformDisplay&&) noexcept = default;

// Move operator.
PlatformDisplay& PlatformDisplay::operator =(PlatformDisplay&&) noexcept = default;

// Presents a rendered image to the user.
//    @param vsync - 'true' to enable vertical synchronization.
void Display::Present(bool vsync)
{
	Fl::lock();
	window->flush();
	Fl::unlock();
	Fl::awake();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//