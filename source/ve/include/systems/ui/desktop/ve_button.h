//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::Button is a convenient widget wrapping Fl_Button functions.
struct Button : public Fl_Box
{
	enum State
	{
		state_release,
		state_push,
		state_hover,
		state_count
	};

	// Constructor, initializes button position, color, text and icon.
	Button(ut::uint32 x,
	       ut::uint32 y,
	       ut::uint32 w,
	       ut::uint32 h);

	// Assigns background color for the provided state.
	void SetBackgroundColor(State state, Fl_Color color);

	// Assigns a callback function that will be triggered
	// on release after push action.
	void SetCallback(ut::Function<void()> new_callback);

	// Assigns a new icon for the button.
	void SetIcon(ut::Array< ut::Color<4, ut::byte> > data,
	             ut::uint32 width,
	             ut::uint32 height);

	// Overrides fltk box behaviour.
	int handle(int e) override;

protected:
	// Struct containing icon image data.
	struct Icon
	{
		ut::Array< ut::Color<4, ut::byte> > data;
		ut::UniquePtr<Fl_RGB_Image> image;
	};

	// Assigns a new state for the button.
	void SetState(State new_state);

	// Current button state.
	State current_state;

	// Background colors for all states.
	Fl_Color bkg_color[state_count];

	// Icon data.
	Icon icon;

	// Callback that is triggered on release after push action.
	ut::Function<void()> callback;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//