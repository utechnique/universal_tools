//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/x.H>
//----------------------------------------------------------------------------//
#ifndef VE_FLTK
#define VE_FLTK UT_FALSE
#endif
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
#if VE_FLTK
//----------------------------------------------------------------------------//
// Converts provided color to the FLTK rgb color.
Fl_Color ConvertToFlColor(const ut::Color<3, ut::byte>& color);

// Converts FLTK rgb color to the UT color.
ut::Color<3, ut::byte> ConvertFlColor(Fl_Color color);

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

//----------------------------------------------------------------------------//
#endif // VE_FLTK
//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
