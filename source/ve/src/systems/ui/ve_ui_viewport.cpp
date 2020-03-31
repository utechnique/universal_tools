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

// ve::ui::Viewport is a polymorphic class, so it must have virtual destructor.
Viewport::~Viewport()
{}

// Connects provided function with signal that is triggered on resize.
void Viewport::ConnectResizeSignalSlot(ut::Function<void(Id, ut::uint32, ut::uint32)> slot)
{
	resize_signal.Connect(ut::Move(slot));
}

// Connects provided function with signal that is triggered in destructor.
void Viewport::ConnectCloseSignalSlot(ut::Function<void(Id id)> slot)
{
	close_signal.Connect(ut::Move(slot));
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
