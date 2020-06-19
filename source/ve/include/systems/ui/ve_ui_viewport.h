//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::Viewport is a special UI window hosting drawable surface.
class Viewport
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
	Viewport(Id viewport_id, ut::String viewport_name);

	// ve::ui::Viewport is a polymorphic class, so it must have virtual destructor.
	virtual ~Viewport() = default;
	
	// Resizes UI widget if resizing is pending.
	virtual void ResizeCanvas();

	// Connects provided function with signal that is triggered on resize.
	void ConnectResize(ut::Function<void(Id id, ut::uint32 w, ut::uint32 h)> slot);

	// Connects provided function with signal that is triggered in destructor.
	void ConnectClose(ut::Function<void(Id id)> slot);

	// Resets all signals.
	void ResetSignals();

	// Returns 'true' if this viewport is currently active
	bool IsActive();

	// Returns unique identifier of the viewport.
	Id GetId() const;

	// Returns name of the viewport.
	const ut::String& GetName() const;

protected:
	// Indicates if this viewport is currently active
	ut::Synchronized<bool> active;

	// Name of the viewport.
	const ut::String name;

	// Unique id associated with this viewport.
	const Id id;

	// Signal that is triggered on resize.
	ut::Signal<void(Id id, ut::uint32 w, ut::uint32 h)> resize_signal;

	// Signal that is triggered in destructor.
	ut::Signal<void(Id id)> close_signal;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//