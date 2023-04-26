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
	friend class DesktopFrontend;
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
	DesktopViewport(Id viewport_id,
	                ut::String viewport_name,
	                int x, int y,
	                int w, int h);

	// Destructors, close signal is triggered here.
    ~DesktopViewport();

	// Returns relative mouse position inside this viewport
	// or nothing if it's outside.
	ut::Optional< ut::Vector<2> > GetCursorPosition() override;

	// Returns relative mouse position offset from the previous
	// frame or nothing if it's outside.
	//    @param reset - boolean indicating if current offset must be reset.
	ut::Optional< ut::Vector<2> > GetCursorOffset(bool reset) override;

	// Assigns new relative mouse position for the current frame.
	void SetMousePosition(ut::Optional< ut::Vector<2> > position);

	// Resizes UI widget if resizing is pending.
	void ResizeCanvas();

private:
	// Updates width and height of the current mode.
	void UpdateSize(int w, int h);

	// Forces viewport to call closure signal.
	void CloseSignal();

	// Overriden virtual function of the base class (Fl_Window).
	// Resize signal is triggered here.
	void resize(int x, int y, int w, int h) override;

	// indicates if resize action is pending and represents a rect with new
	// position and size of the window
	ut::Optional< ut::Rect<int> > resize_task;

	// relative mouse position for the current frame:
	// center is (0,0), X is right, Y is up, distance to the border is 1
	ut::Synchronized< ut::Optional< ut::Vector<2> > > cur_mouse_position;
	ut::Synchronized< ut::Optional< ut::Vector<2> > > prev_mouse_position;
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
