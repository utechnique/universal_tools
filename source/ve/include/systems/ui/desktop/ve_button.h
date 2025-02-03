//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_icon.h"
//----------------------------------------------------------------------------//
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::Button is a convenient widget wrapping Fl_Button functions.
class Button : public Fl_Box
{
public:
	enum class State
	{
		release,
		push,
		hover,
		count
	};

	// Constructor, initializes button position.
	Button(ut::uint32 x,
	       ut::uint32 y,
	       ut::uint32 w,
	       ut::uint32 h,
	       ut::String button_text = ut::String());

	// Assigns background color for the provided state.
	void SetBackgroundColor(State state, Fl_Color color);

	// Assigns a callback function that will be triggered
	// on release after push action.
	void SetCallback(ut::Function<void()> new_callback);

	// Assigns a new icon for the button.
	void SetIcon(ut::SharedPtr<Icon> icon_ptr);

	// Returns the current state of the button.
	State GetState() const;

	// Assigns a new state for the button.
	void SetState(State new_state);

protected:
	// Overrides fltk box behaviour.
	virtual int handle(int e) override;

	// Current button state.
	State current_state;

	// Button label.
	ut::UniquePtr<ut::String> text;

	// Background colors for all states.
	Fl_Color bkg_color[static_cast<size_t>(State::count)];

	// Icon data.
	ut::SharedPtr<Icon> icon;

	// Callback that is triggered on release after push action.
	ut::Function<void()> callback;
};

//----------------------------------------------------------------------------//
// ve::ui::BinaryButton is a convenient button widget that can be turned on/off.
class BinaryButton : public Button
{
public:
	// Constructor, initializes button position.
	BinaryButton(ut::uint32 x,
	             ut::uint32 y,
	             ut::uint32 w,
	             ut::uint32 h);

	// Assigns background color for the provided state.
	void SetBackgroundColor(State state, Fl_Color color);

	// Assigns a callback function that will be triggered
	// when the button is turned on.
	void SetOnCallback(ut::Function<void()> new_callback);

	// Assigns a callback function that will be triggered
	// when the button is turned off.
	void SetOffCallback(ut::Function<void()> new_callback);

	// Assigns a new icon for the button.
	void SetOnIcon(ut::SharedPtr<Icon> icon_ptr);

	// Assigns a new icon for the button.
	void SetOffIcon(ut::SharedPtr<Icon> icon_ptr);

	// Returns true is the button is turned on.
	bool IsOn() const;

	// Switches button state.
	void Set(bool new_status, bool callback_trigger = true);

private:
	// "Off" icon data.
	ut::SharedPtr<Icon> off_icon;

	// Callback that is triggered on release after push action.
	ut::Function<void()> on_callback;
	ut::Function<void()> off_callback;

	// Flag indicating that the button is in expanded state.
	bool status;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//