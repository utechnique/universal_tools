//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::JustifyInput is a special widget supporting the alignment for the
// input field. By default this widget represents a simple box, but when a user
// clicks on it - it becomes an input widget.
template<typename BaseWidget = Fl_Input>
class JustifyInput : public Fl_Group
{
	ut::UniquePtr<BaseWidget> inp;
	ut::UniquePtr<Fl_Box> box;
	ut::UniquePtr<ut::String> buffer;
public:
	JustifyInput(int in_x,
	             int in_y,
	             int in_w,
	             int in_h,
	             const char *in_label = 0) : Fl_Group(in_x, in_y,
	                                                  in_w, in_h,
	                                                  in_label)
	{
		Fl_Group::box(FL_NO_BOX);
		box = ut::MakeUnique<Fl_Box>(in_x, in_y, in_w, in_h);
		box->box(FL_DOWN_BOX);
		inp = ut::MakeUnique<Fl_Int_Input>(in_x, in_y, in_w, in_h);
		inp->hide();
		end();
	}

	void align(Fl_Align val)
	{
		box->align(val | FL_ALIGN_INSIDE);
	}

	void value(const char *val)
	{
		buffer = ut::MakeUnique<ut::String>(val);
		box->label(buffer->GetAddress());
		inp->value(buffer->GetAddress());
	}

	const char* value() const
	{
		return inp->value();
	}

	const BaseWidget* input() const
	{
		return inp.Get();
	}

	BaseWidget* input()
	{
		return inp.Get();
	}

	void labelfont(Fl_Font val)
	{
		box->labelfont(val);
		inp->textfont(val);
	}

	void labelsize(int label_size)
	{
		box->labelsize(label_size);
		inp->textsize(label_size);
	}

	int handle(int e)
	{
		switch (e)
		{
		case FL_PUSH:
		case FL_FOCUS:
			if (!inp->visible())
			{
				box->hide();
				inp->show();
				inp->value(box->label());
				redraw();
			}
			break;
		case FL_UNFOCUS:
			if (inp->visible())
			{
				box->show();
				inp->hide();
				box->label(inp->value());
				redraw();
			}
			break;
		default:
			break;
		}
		return(Fl_Group::handle(e));
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//