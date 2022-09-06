//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
#if VE_FLTK
// Converts provided color to the FLTK rgb color.
Fl_Color ConvertToFlColor(const ut::Color<3, ut::byte>& color)
{
	return fl_rgb_color(color.R(), color.G(), color.B());
}

// Helper function to find absolute position of the widget.
ut::Vector<2, int> AccumulateFlAbsOffset(Fl_Widget* widget)
{
	if (widget == nullptr)
	{
		return ut::Vector<2, int>(0, 0);
	}

	Fl_Group* parent = widget->parent();
	if (parent == nullptr)
	{
		return ut::Vector<2, int>(widget->x(), widget->y());
	}

	const bool is_a_window = widget->as_window() != nullptr;
	const ut::Vector<2, int> offset = is_a_window ?
	                                  ut::Vector<2, int>(widget->x(), widget->y()) :
	                                  ut::Vector<2, int>(0, 0);

	return offset + AccumulateFlAbsOffset(parent);
}

// Returns the absolute (screen) postition of the provided widget.
ut::Vector<2, int> GetFlAbsPosition(Fl_Widget* widget)
{
	return ut::Vector<2, int>(widget->x(), widget->y()) + AccumulateFlAbsOffset(widget->parent());
}
#endif // VE_FLTK
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
