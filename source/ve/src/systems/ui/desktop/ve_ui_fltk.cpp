//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
#if VE_FLTK
//----------------------------------------------------------------------------//
// Converts provided color to the FLTK rgb color.
Fl_Color ConvertToFlColor(const ut::Color<3, ut::byte>& color)
{
	return fl_rgb_color(color.R(), color.G(), color.B());
}

// Helper function to find absolute position of the widget.
ut::Vector<2, int> AccumulateFlAbsOffset(Fl_Widget* widget,
                                         Fl_Widget* final_parent)
{
	if (widget == nullptr)
	{
		return ut::Vector<2, int>(0, 0);
	}

	Fl_Group* parent = widget->parent();
	if (parent == nullptr || parent == final_parent)
	{
		return final_parent == nullptr ? ut::Vector<2, int>(widget->x(), widget->y()) :
		                                 ut::Vector<2, int>(0, 0);
	}

	const bool is_a_window = widget->as_window() != nullptr;
	const ut::Vector<2, int> offset = is_a_window ?
	                                  ut::Vector<2, int>(widget->x(), widget->y()) :
	                                  ut::Vector<2, int>(0, 0);

	return offset + AccumulateFlAbsOffset(parent, final_parent);
}

// Returns the absolute (screen) postition of the provided widget.
ut::Vector<2, int> GetFlAbsPosition(Fl_Widget* widget,
                                    Fl_Widget* final_parent)
{
	return ut::Vector<2, int>(widget->x(), widget->y()) +
	                          AccumulateFlAbsOffset(widget->parent(), final_parent);
}

// Pass a callback to the constructor to execute it in the fltk thread
// and wait for completion.
FltkSync::FltkSync(ut::Function<void()> fltk_callback) : flag(false)
                                                       , callback(ut::Move(fltk_callback))
{
	Fl::awake([](void* ptr) { static_cast<FltkSync*>(ptr)->Finish(); }, this);
	Wait();
}

// Executes a callback in the fltk thread and signals avout completion.
void FltkSync::Finish()
{
	callback();
	ut::ScopeLock lock(mutex);
	flag = true;
	cvar.WakeOne();
}

// Waits for a callback to finish.
void FltkSync::Wait()
{
	ut::ScopeLock lock(mutex);
	while (!flag)
	{
		cvar.Wait(lock);
	}
}

//----------------------------------------------------------------------------//
#endif // VE_FLTK
//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
