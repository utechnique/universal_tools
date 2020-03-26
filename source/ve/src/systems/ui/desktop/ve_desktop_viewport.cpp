//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_viewport.h"
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
Viewport::Viewport(Id viewport_id,
	               ut::String viewport_name,
	               int x, int y,
                   int w, int h) : Fl_Window(x, y, w, h)
	                             , id(viewport_id)
	                             , name(ut::Move(viewport_name))
{}

// Connects provided function with signal that is triggered on resize.
void Viewport::ConnectResizeSignalSlot(ut::Function<void(Id, ut::uint32, ut::uint32)> slot)
{
	resize_signal.Connect(ut::Move(slot));
}

// Returns unique identifier of the viewport.
Viewport::Id Viewport::GetId() const
{
	return id;
}

// Overriden virtual function of the base class (Fl_Window).
// Resize signal is triggered here.
void Viewport::resize(int x, int y, int w, int h)
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