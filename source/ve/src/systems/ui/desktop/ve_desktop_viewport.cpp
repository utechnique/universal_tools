//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor.
//    @param viewport_id - id associated with this viewport.
//    @param viewport_name - name of the viewport.
//    @param x - the initial horizontal position of the window in pixels.
//    @param y - the initial vertical position of the window in pixels.
//    @param w - the initial width of the window in pixels.
//    @param h - the initial height of the window in pixels.
DesktopViewport::DesktopViewport(Id viewport_id,
                                 ut::String viewport_name,
                                 int x, int y,
                                 int w, int h) : Viewport(viewport_id, ut::Move(viewport_name))
                                               , Fl_Window(x, y, w, h)
{}

// Destructors, close signal is triggered here.
DesktopViewport::~DesktopViewport()
{
    close_signal(id);
}

// Overriden virtual function of the base class (Fl_Window).
// Resize signal is triggered here.
void DesktopViewport::resize(int x, int y, int w, int h)
{
	Fl_Window::resize(x, y, w, h);
	resize_signal(id, w, h);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
