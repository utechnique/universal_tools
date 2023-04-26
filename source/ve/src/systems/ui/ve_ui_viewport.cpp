//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_viewport.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor.
//    @param viewport_id - id associated with this viewport.
//    @param viewport_name - name of the viewport.
Viewport::Viewport(Id viewport_id,
                   ut::String viewport_name) : id(viewport_id)
                                             , name(ut::Move(viewport_name))
{}

// Resizes UI widget if resizing is pending.
void Viewport::ResizeCanvas()
{}

// Connects provided function with signal that is triggered on resize.
void Viewport::ConnectResize(ut::Function<void(Id, ut::uint32, ut::uint32)> slot)
{
	resize_signal.Connect(ut::Move(slot));
}

// Connects provided function with signal that is triggered in destructor.
void Viewport::ConnectClose(ut::Function<void(Id id)> slot)
{
	close_signal.Connect(ut::Move(slot));
}

// Resets all signals.
void Viewport::ResetSignals()
{
	resize_signal.Reset();
	close_signal.Reset();
}

// Returns current mode. Thread-safe.
Viewport::Mode Viewport::GetMode()
{
	return mode.Get();
}

// Sets new mode. Thread-safe.
void Viewport::SetMode(const Viewport::Mode& new_mode)
{
	mode.Set(new_mode);
}

// Returns relative mouse position inside this viewport
// or nothing if it's outside. Position (0,0) is located
// in the center of the viewport, X-axis is right, Y-axis
// is up. Distance to the viewport border is 1.
ut::Optional< ut::Vector<2> > Viewport::GetCursorPosition()
{
	return ut::Optional< ut::Vector<2> >();
}

// Returns relative mouse position offset from the previous
// frame or nothing if it's outside.
//    @param reset - boolean indicating if current offset must be reset.
ut::Optional< ut::Vector<2> > Viewport::GetCursorOffset(bool reset)
{
	return ut::Optional< ut::Vector<2> >();
}

// Returns unique identifier of the viewport.
Viewport::Id Viewport::GetId() const
{
	return id;
}

// Returns name of the viewport.
const ut::String& Viewport::GetName() const
{
	return name;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
