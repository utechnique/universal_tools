//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Locks UI display and makes widget surface accessible.
void LockDisplay()
{
#if VE_X11
	XLockDisplay(fl_display);
#endif
}

// Unlocks UI display and returns control over the widget
// surface to the ui thread.
void UnlockDisplay()
{
#if VE_X11
	XUnlockDisplay(fl_display);
#endif
}

// Constructor.
DisplayScopeLock::DisplayScopeLock()
{
	LockDisplay();
}

// Destructor.
DisplayScopeLock::~DisplayScopeLock()
{
	UnlockDisplay();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
