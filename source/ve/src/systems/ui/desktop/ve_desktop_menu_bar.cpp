//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_entity_browser.h"
#include "systems/ui/desktop/ve_desktop_menu_bar.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Height of the bar in pixels.
const ut::uint32 MenuBar::skHeight = 24;

//----------------------------------------------------------------------------//


void MenuCallbacks::ShowEntityBrowser(Fl_Widget* menu, void* ptr)
{
	EntityBrowser* browser = static_cast<EntityBrowser*>(ptr);
	browser->show();
	browser->take_focus();
}

// Constructor.
MenuBar::MenuBar(const Settings& settings,
                 ut::uint32 x,
                 ut::uint32 y,
                 ut::uint32 width,
                 EntityBrowser& in_entity_browser) : Fl_Menu_Bar(x, y, width, skHeight)
                                                   , entity_browser(in_entity_browser)
{
	box(FL_FLAT_BOX);

	const Fl_Menu_Item menu_table[] =
	{
		{ "File", 0, 0, 0, FL_SUBMENU },
			{ "Open", 0, 0 },
			{ "Save", 0, 0, 0, FL_MENU_DIVIDER },
			{ "Quit", 0, 0, },
			{ 0 },
		{ "Edit", FL_F + 2, 0, 0, FL_SUBMENU },
			{ "Select entities", 0, MenuCallbacks::ShowEntityBrowser, &entity_browser },
			{ 0 },
		{ 0 }
	};

	const size_t item_count = sizeof(menu_table) / sizeof(menu_table[0]);
	items.Resize(item_count);
	for (size_t i = 0; i < item_count; i++)
	{
		items[i] = menu_table[i];
	}

	menu(items.GetAddress());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
