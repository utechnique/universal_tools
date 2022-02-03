//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
#include <FL/Fl_Scroll.H>
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Helper widget to perform correct scrolling while instantly updating content.
// This class exists beacause Fl_Scroll sets scrolling position to zero after
// all children are removed. And it does this so that it's impossible to update 
// contents in real time preserving current scrolling position.
class Scroll : public Fl_Scroll
{
public:
	Scroll(int x, int y, int w, int h);
	virtual ~Scroll() override = default;
	void draw() override;
	ut::Optional< ut::Vector<2, int> > scroll_position;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//