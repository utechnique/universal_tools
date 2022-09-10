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

// Simple synchronization unit to synchronize fltk thread using Fl::awake() call.
class FltkSync
{
public:
	// Pass a callback to the constructor to execute it in the fltk thread
	// and wait for completion.
	FltkSync(ut::Function<void()> fltk_callback);

private:
	// Executes a callback in the fltk thread and signals avout completion.
	void Finish();

	// Waits for a callback to finish.
	void Wait();

	ut::ConditionVariable cvar;
	ut::Mutex mutex;
	bool flag;
	ut::Function<void()> callback;
};
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
