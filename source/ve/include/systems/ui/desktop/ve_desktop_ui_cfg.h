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

	// colors
	ut::Color<3, ut::uint32> background_color;
	ut::Color<3, ut::uint32> foreground_color;
	ut::Color<3, ut::uint32> tab_color;
	ut::Color<3, ut::uint32> viewport_hover_color;
	ut::Color<3, ut::uint32> viewport_focus_color;

	// main window
	ut::Rect<ut::uint32> window;

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
