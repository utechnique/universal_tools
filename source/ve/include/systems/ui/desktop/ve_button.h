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
class Button : public Fl_Box
{
public:
	enum State
	{
		state_release,
		state_push,
		state_hover,
		state_count
	};

	// Constructor, initializes button position.
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

protected:
	// Struct containing icon image data.
	struct Icon
	{
		ut::Array< ut::Color<4, ut::byte> > data;
		ut::UniquePtr<Fl_RGB_Image> image;
	};

	// Overrides fltk box behaviour.
	virtual int handle(int e) override;

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
// ve::ui::ExpandButton is a convenient widget to control expansion and 
// collapsing of different UI lists / trees.
class ExpandButton : public Button
{
public:
	// Constructor, initializes button position.
	ExpandButton(ut::uint32 x,
	             ut::uint32 y,
	             ut::uint32 w,
	             ut::uint32 h,
	             const ut::Color<4, ut::byte>& icon_color);

	// Assigns background color for the provided state.
	void SetBackgroundColor(State state, Fl_Color color);

	// Assigns a callback function that will be triggered
	// on expand action.
	void SetExpandCallback(ut::Function<void()> new_callback);

	// Assigns a callback function that will be triggered
	// on collapse action.
	void SetCollapseCallback(ut::Function<void()> new_callback);

	// Returns true is the button is in the expanded state.
	bool IsExpanded() const;

private:
	// Switches from expanded to collapsed state and vice versa.
	void SwitchExpansionMode();

	// Creates icon data for the collapse icon.
	static ut::Array< ut::Color<4, ut::byte> > CreateCollapseIcon(bool expanded,
	                                                              const ut::Color<4, ut::byte>& icon_color,
	                                                              ut::uint32 width,
	                                                              ut::uint32 height);

	// Collapse icon data.
	Icon collapse_icon;

	// Callback that is triggered on release after push action.
	ut::Function<void()> expand_callback;
	ut::Function<void()> collapse_callback;

	// Flag indicating that the button is in expanded state.
	bool is_expanded;

};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//