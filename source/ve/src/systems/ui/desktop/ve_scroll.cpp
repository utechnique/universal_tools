//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_scroll.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor.
Scroll::Scroll(int x, int y, int w, int h) : Fl_Scroll(x, y, w, h)
{}

// Ovverides base class and performs additional reposition after drawing.
void Scroll::draw()
{
	// internal scroll position is updated here (it will become invalid after
	// the views are updated)
	Fl_Scroll::draw();

	// set back the original scrolling position
	if (scroll_position)
	{
		scroll_to(scroll_position->X(), scroll_position->Y());
		scroll_position.Empty();
	}

	// whole group must be redrawn to apply changes
	Fl_Scroll::draw();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
