//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_entity_selector.h"
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Sys_Menu_Bar.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
struct MenuCallbacks
{
	static void ShowEntitySelector(Fl_Widget* menu, void* selector);
};

// Menu bar.
class MenuBar : public Fl_Menu_Bar
{
	friend MenuCallbacks;
public:
	// Constructor.
	MenuBar(const Settings& settings,
	        ut::uint32 x,
	        ut::uint32 y,
	        ut::uint32 width,
	        EntitySelector& in_entity_selector);

	// Height of the bar in pixels.
	static const ut::uint32 skHeight;

private:
	ut::Array<Fl_Menu_Item> items;
	EntitySelector& entity_selector;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//