//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
#include "ve_config.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Color scheme.
class Theme : public ut::meta::Reflective
{
public:
	enum class ColorScheme
	{
		dark,
		light
	};

	// Constructor, creates a theme with the provided color scheme.
	Theme(ColorScheme scheme = ColorScheme::dark);

	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	// Replaces original FLTK color scheme with own colors.
	void ApplyToFltk() const;

	// colors
	ut::Color<3, ut::byte> background_color;
	ut::Color<3, ut::byte> foreground_color;
	ut::Color<3, ut::byte> frame_color;
	ut::Color<3, ut::byte> primary_tab_color;
	ut::Color<3, ut::byte> secondary_tab_color;
	ut::Color<3, ut::byte> button_hover_color;
	ut::Color<3, ut::byte> button_push_color;
	ut::Color<3, ut::byte> input_color;
	ut::Color<3, ut::byte> viewport_hover_color;
	ut::Color<3, ut::byte> viewport_focus_color;
};

//----------------------------------------------------------------------------//
// Hotkey bindings.
class HotkeyBindings : public ut::meta::Reflective
{
public:
	// Constructor, initializes hotkey bindings.
	HotkeyBindings();

	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	// bindings
	ut::String show_properties;
};

//----------------------------------------------------------------------------//
// ve::ui::Settings is a class containing options and
// preferences for the desktop ui widgets.
class Settings : public ut::meta::Reflective
{
public:
	// Constructor, default values are set here.
	Settings();

	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	// color theme
	Config<Theme> theme;

	// hot keys bindings
	HotkeyBindings hotkeys;

	// main window
	ut::Rect<ut::uint32> window;
	bool maximized;

	// size of the frame between viewports in pixels
	ut::uint32 viewport_frame_size;

	// selected layout
	ut::uint32 layout_id;

	// viewports
	ut::Array<ut::uint32> projections;
	ut::Array< ut::Rect<ut::uint32> > viewports;
	ut::Array<ut::uint32> render_modes;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
