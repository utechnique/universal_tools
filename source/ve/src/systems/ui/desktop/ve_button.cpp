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
// Constructor, initializes button position, color, text and icon.
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
void Button::SetIcon(ut::Array< ut::Color<4, ut::byte> > data,
                     ut::uint32 width,
                     ut::uint32 height)
{
	icon.data = ut::Move(data);
	icon.image = ut::MakeUnique<Fl_RGB_Image>(icon.data.GetFirst().GetData(),
	                                          width,
	                                          height,
	                                          4);
	this->image(icon.image.Get());
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
		SetState(state_release);
		if (callback.IsValid() && ex > x() && ey > y() && ex < x() + w() && ey < y() + h())
		{
			callback();
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
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
