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
               ut::uint32 h,
               ut::String button_text) : Fl_Box(x, y, w, h)
                                       , text(ut::MakeUnique<ut::String>(ut::Move(button_text)))
                                       , bkg_color{ FL_BACKGROUND_COLOR,
                                                    fl_color_average(FL_BACKGROUND_COLOR, FL_FOREGROUND_COLOR, 0.75f),
                                                    fl_color_average(FL_BACKGROUND_COLOR, FL_FOREGROUND_COLOR, 0.75f) }
{
	box(FL_FLAT_BOX);
	SetState(State::release);
	if (text->Length() != 0)
	{
		label(text->GetAddress());
	}
}

// Assigns background color for the provided state.
void Button::SetBackgroundColor(State state, Fl_Color new_color)
{
	UT_ASSERT(state < State::count);
	bkg_color[static_cast<size_t>(state)] = new_color;
	color(bkg_color[static_cast<size_t>(current_state)]);
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

// Assigns a new state for the button.
Button::State Button::GetState() const
{
	return current_state;
}

// Assigns a new state for the button.
void Button::SetState(State new_state)
{
	current_state = new_state;
	color(bkg_color[static_cast<size_t>(new_state)]);
	redraw();
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
		SetState(State::push);
		return 1;
	case FL_RELEASE:
		if (ex >= x() && ey >= y() && ex < x() + w() && ey < y() + h())
		{
			if (callback.IsValid())
			{
				callback();
			}
			SetState(State::hover);
		}
		else
		{
			SetState(State::release);
		}
		return 1;
	case FL_MOVE:
		SetState(State::hover);
		return 1;
	case FL_ENTER:
		SetState(State::hover);
		return 1;
	case FL_LEAVE:
		SetState(State::release);
		return 1;
	case FL_SHOW:
		SetState(State::release);
		return 1;
	}

	return ret;
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
void BinaryButton::Set(bool new_status, bool callback_trigger)
{
	Fl_RGB_Image* const off_img = off_icon.Get() ? off_icon->GetImage() : nullptr;
	Fl_RGB_Image* const on_img = Button::icon.Get() ? Button::icon->GetImage() : nullptr;
	Fl_RGB_Image* const img = new_status ? on_img : off_img;

	status = new_status;
	Fl_Box::image(img);
	redraw();

	if (callback_trigger)
	{
		ut::Function<void()>& callback = new_status ? on_callback : off_callback;
		callback();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
