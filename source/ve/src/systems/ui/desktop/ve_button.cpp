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
void Button::SetIcon(ut::Array< ut::Color<4, ut::byte> > data,
                     ut::uint32 width,
                     ut::uint32 height)
{
	icon.data = ut::Move(data);
	icon.image = ut::MakeUnique<Fl_RGB_Image>(icon.data.GetFirst().GetData(),
	                                          width,
	                                          height,
	                                          4);
	Fl_Box::image(icon.image.Get());
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
// Constructor, initializes button position.
ExpandButton::ExpandButton(ut::uint32 x,
                           ut::uint32 y,
                           ut::uint32 w,
                           ut::uint32 h,
                           const ut::Color<4, ut::byte>& icon_color) : Button(x, y, w, h)
                                                                     , is_expanded(false)
{
	// create icons
	Button::icon.data = CreateCollapseIcon(false, icon_color, w, h);
	collapse_icon.data = CreateCollapseIcon(true, icon_color, w, h);
	Button::icon.image = ut::MakeUnique<Fl_RGB_Image>(Button::icon.data.GetFirst().GetData(), w, h, 4);
	collapse_icon.image = ut::MakeUnique<Fl_RGB_Image>(collapse_icon.data.GetFirst().GetData(), w, h, 4);
	Fl_Box::image(Button::icon.image.Get());

	// set parent callback
	Button::SetCallback([&] {this->SwitchExpansionMode(); });
}

// Assigns background color for the provided state.
void ExpandButton::SetBackgroundColor(State state, Fl_Color color)
{
	Button::SetBackgroundColor(state, color);
}

// Assigns a callback function that will be triggered
// on expand action.
void ExpandButton::SetExpandCallback(ut::Function<void()> new_callback)
{
	expand_callback = ut::Move(new_callback);
}

// Assigns a callback function that will be triggered
// on collapse action.
void ExpandButton::SetCollapseCallback(ut::Function<void()> new_callback)
{
	collapse_callback = ut::Move(new_callback);
}

// Returns true is the button is in the expanded state.
bool ExpandButton::IsExpanded() const
{
	return is_expanded;
}

// Switches from expanded to collapsed state and vice versa.
void ExpandButton::SwitchExpansionMode()
{
	if (is_expanded)
	{
		is_expanded = false;
		Fl_Box::image(Button::icon.image.Get());
		redraw();
		if (collapse_callback.IsValid())
		{
			collapse_callback();
		}
	}
	else
	{
		is_expanded = true;
		Fl_Box::image(collapse_icon.image.Get());
		redraw();
		if (expand_callback.IsValid())
		{
			expand_callback();
		}
	}
}

// Creates icon data for the collapse icon.
ut::Array< ut::Color<4, ut::byte> > ExpandButton::CreateCollapseIcon(bool expanded,
                                                                     const ut::Color<4, ut::byte>& icon_color,
                                                                     ut::uint32 width,
                                                                     ut::uint32 height)
{
	ut::Array< ut::Color<4, ut::byte> > icon_data(width * height);

	const int size = ut::Min<ut::uint32>(width, height);
	const int half_size = size / 2;
	const int margin = size / 4;
	const int inv_margin = size - margin - 1;
	const bool odd = size % 2 == 0 ? true : false;

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			ut::Color<4, ut::byte>& pixel = icon_data[y*size + x];
			pixel = icon_color;
			pixel.A() = 0;

			const bool inside = x >= margin &&
			                    x <= inv_margin &&
			                    y >= margin &&
			                    y <= inv_margin;
			if (!inside)
			{
				continue;
			}

			const int oy = y + size / (expanded ?  6 : -6);

			if (!expanded && oy > half_size)
			{
				continue;
			}
			else if (!expanded && oy == margin - 1)
			{
				pixel.A() = icon_color.A();
				continue;
			}
			else if (expanded && oy < half_size)
			{
				continue;
			}

			const int line_width = 1;
			const int d0 = ut::Abs(oy - x);
			const int d1 = ut::Abs(oy - size + x + 1);
			
			if (odd && oy == half_size)
			{
				const int ix = x - half_size;
				if (ix == 0 || ix == -1)
				{
					pixel.A() = icon_color.A() / 2;
				}
				continue;
			}
			
			if (d0 < line_width || d1 < line_width)
			{
				pixel.A() = icon_color.A();
			}
			else if (d0 < line_width + 1 || d1 < line_width + 1)
			{
				pixel.A() = icon_color.A() / 2;
			}
		}
	}

	return icon_data;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
