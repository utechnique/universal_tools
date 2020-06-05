//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
#define VE_DESKTOP UT_TRUE

// fltk is the only ui framework supported (for now) for desktops
#define VE_FLTK VE_DESKTOP
#if VE_FLTK
#include <FL/Fl.H>
#include <FL/x.H>
#endif

// fltk uses XLib for linux
#define VE_X11 VE_FLTK && UT_LINUX

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Some platforms may require UI thread to stop, so that another system
// (like rendering) could draw into the desired widget.
// This function locks UI display and makes widget surface accessible.
void LockDisplay();

// Unlocks UI display and returns control over the widget
// surface to the ui thread.
void UnlockDisplay();

// Calls ve::ui::LockDisplay() in constructor and ve::ui::UnlockDisplay()
// in destructor.
class DisplayScopeLock
{
public:
	DisplayScopeLock();
	~DisplayScopeLock();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
