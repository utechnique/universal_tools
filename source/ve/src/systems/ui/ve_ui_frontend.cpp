//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_frontend.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Title of the application.
const char* Frontend::skTitle = "Virtual Environment";

// When ui exits.
void Frontend::ConnectExitSignalSlot(ut::Function<void()> slot)
{
	exit_signal.Connect(slot);
}

// One can start iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::Iterator Frontend::BeginViewports()
{
	return viewports.Begin();
}

// One can end iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::Iterator Frontend::EndViewports()
{
	return viewports.End();
}

// One can start iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::ConstIterator Frontend::BeginViewports() const
{
	return viewports.Begin();
}

// One can end iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::ConstIterator Frontend::EndViewports() const
{
	return viewports.End();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//