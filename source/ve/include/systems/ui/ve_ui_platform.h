//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
#define VE_DESKTOP UT_TRUE

// FLTK is the only ui framework supported (for now) for the desktop platform.
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
#if VE_FLTK
#include <FL/Fl.H>
#include <FL/x.H>

// Converts provided color to the FLTK rgb color.
Fl_Color ConvertToFlColor(const ut::Color<3, ut::byte>& color);

// Returns the absolute (screen) postition of the provided widget.
ut::Vector<2, int> GetFlAbsPosition(Fl_Widget* widget,
                                    Fl_Widget* final_parent = nullptr);
#endif
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
