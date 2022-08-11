//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_button.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor, initializes button position.
Button::Button(ut::uint32 x,
               ut::uint32 y,
               ut::uint32 w,
               ut::uint32 h) : Fl_Box(x, y, w, h)
                             , bkg_color{ FL_BACKGROUND_COLOR,
                                          FL_BACKGROUND_COLOR,
                                          FL_BACKGROUND_COLOR }
{
	box(FL_FLAT_BOX);
	SetState(state_release);
}

// Assigns background color for the provided state.
void Button::SetBackgroundColor(State state, Fl_Color new_color)
{
	UT_ASSERT(state < state_count);
	bkg_color[state] = new_color;
	color(bkg_color[current_state]);
}

// Assigns a callback function that will be triggered on release action.
void Button::SetCallback(ut::Function<void()> new_callback)
{
	callback = ut::Move(new_callback);
}

// Assigns a new icon for the button.
void Button::SetIcon(ut::SharedPtr<Icon> icon_ptr)
{
	icon = ut::Move(icon_ptr);
	Fl_Box::image(icon->GetImage());
}

// Overrides fltk box behaviour.
int Button::handle(int e)
{
	int ret = Fl_Box::handle(e);
	const int ex = Fl::event_x();
	const int ey = Fl::event_y();

	switch (e)
	{
	case FL_PUSH:
		SetState(state_push);
		return 1;
	case FL_RELEASE:
		if (ex > x() && ey > y() && ex < x() + w() && ey < y() + h())
		{
			if (callback.IsValid())
			{
				callback();
			}
			SetState(state_hover);
		}
		else
		{
			SetState(state_release);
		}
		return 1;
	case FL_MOVE:
		SetState(state_hover);
		return 1;
	case FL_ENTER:
		SetState(state_hover);
		return 1;
	case FL_LEAVE:
		SetState(state_release);
		return 1;
	case FL_SHOW:
		SetState(state_release);
		return 1;
	}

	return ret;
}

// Assigns a new state for the button.
void Button::SetState(State new_state)
{
	current_state = new_state;
	color(bkg_color[new_state]);
	redraw();
}

//----------------------------------------------------------------------------//
// Constructor, initializes button position.
BinaryButton::BinaryButton(ut::uint32 x,
                           ut::uint32 y,
                           ut::uint32 w,
                           ut::uint32 h) : Button(x, y, w, h)
                                         , status(false)
{
	// set parent callback
	Button::SetCallback([&] {this->Set(!status); });
}

// Assigns background color for the provided state.
void BinaryButton::SetBackgroundColor(State state, Fl_Color color)
{
	Button::SetBackgroundColor(state, color);
}

// Assigns a callback function that will be triggered
// on expand action.
void BinaryButton::SetOnCallback(ut::Function<void()> new_callback)
{
	on_callback = ut::Move(new_callback);
}

// Assigns a callback function that will be triggered
// on collapse action.
void BinaryButton::SetOffCallback(ut::Function<void()> new_callback)
{
	off_callback = ut::Move(new_callback);
}

// Assigns a new icon for the button.
void BinaryButton::SetOnIcon(ut::SharedPtr<Icon> icon_ptr)
{
	Button::icon = ut::Move(icon_ptr);
	if (status)
	{
		Fl_Box::image(Button::icon->GetImage());
	}
}

// Assigns a new icon for the button.
void BinaryButton::SetOffIcon(ut::SharedPtr<Icon> icon_ptr)
{
	off_icon = ut::Move(icon_ptr);
	if (!status)
	{
		Fl_Box::image(off_icon->GetImage());
	}
}

// Returns true is the button is in the expanded state.
bool BinaryButton::IsOn() const
{
	return status;
}

// Switches from expanded to collapsed state and vice versa.
void BinaryButton::Set(bool new_status)
{
	Fl_RGB_Image* const off_img = off_icon.Get() ? off_icon->GetImage() : nullptr;
	Fl_RGB_Image* const on_img = Button::icon.Get() ? Button::icon->GetImage() : nullptr;
	Fl_RGB_Image* const img = new_status ? on_img : off_img;
	ut::Function<void()>& callback = new_status ? on_callback : off_callback;

	status = new_status;
	Fl_Box::image(img);
	redraw();
	callback();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
