//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_viewport.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Window.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::DestopViewport is a viewport for desktop applications.
class DesktopViewport : public Viewport, public Fl_Window
{
public:
	// Every viewport must have unique id.
	typedef ut::uint32 Id;

	// Constructor.
	//    @param viewport_id - id associated with this viewport.
	//    @param viewport_name - name of the viewport.
	//    @param x - the initial horizontal position of the window in pixels.
	//    @param y - the initial vertical position of the window in pixels.
	//    @param w - the initial width of the window in pixels.
	//    @param h - the initial height of the window in pixels.
	DesktopViewport(Id viewport_id, ut::String viewport_name, int x, int y, int w, int h);

	// Destructors, close signal is triggered here.
    ~DesktopViewport();

private:

	// Overriden virtual function of the base class (Fl_Window).
	// Resize signal is triggered here.
	void resize(int x, int y, int w, int h) override;
};

// UI viewport for the current platform.
typedef DesktopViewport PlatformViewport;

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
