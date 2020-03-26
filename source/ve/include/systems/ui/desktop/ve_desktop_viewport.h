//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::Viewport is a special UI window hosting 3D rendering context.
class Viewport : public Fl_Window
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
	Viewport(Id viewport_id, ut::String viewport_name, int x, int y, int w, int h);

	// Connects provided function with signal that is triggered on resize.
	void ConnectResizeSignalSlot(ut::Function<void(Id id, ut::uint32 w, ut::uint32 h)> slot);

	// Returns unique identifier of the viewport.
	Id GetId() const;

private:
	// Overriden virtual function of the base class (Fl_Window).
	// Resize signal is triggered here.
	void resize(int x, int y, int w, int h) override;

	// Name of the viewport.
	const ut::String name;

	// Unique id associated with this viewport.
	const Id id;

	// Signal that is triggered on resize.
	ut::Signal<void(Id id, ut::uint32 w, ut::uint32 h)> resize_signal;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//