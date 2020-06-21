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
                   ut::String viewport_name) : active(true)
                                             , id(viewport_id)
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

// Returns 'true' if this viewport is currently active
bool Viewport::IsActive()
{
	return active.Get();
}

// Activates this viewport.
void Viewport::Activate()
{
	active.Set(true);
}

// Deactivates this viewport
void Viewport::Deactivate()
{
	active.Set(false);
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
