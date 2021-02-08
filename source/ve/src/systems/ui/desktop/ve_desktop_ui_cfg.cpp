//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
#include "systems/ui/ve_ui_viewport.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
template<> const char* ve::Config<ve::ui::Settings>::skName = "ui";
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : background_color(51, 51, 55)
                     , foreground_color(150, 150, 165)
                     , tab_color(80, 80, 88)
                     , viewport_hover_color(115, 115, 120)
                     , viewport_focus_color(192, 175, 23)
                     , window(0, 0, 640, 480)
                     , viewport_frame_size(4)
                     , layout_id(0)
{
	projections.Add(Viewport::perspective);
	projections.Add(Viewport::orthographic_negative_y);
	projections.Add(Viewport::orthographic_positive_x);
	projections.Add(Viewport::orthographic_positive_z);
}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	// colors
	snapshot.Add(background_color, "background_color");
	snapshot.Add(foreground_color, "foreground_color");
	snapshot.Add(tab_color, "tab_color");
	snapshot.Add(viewport_hover_color, "viewport_hover_color");
	snapshot.Add(viewport_focus_color, "viewport_focus_color");

	// main window
	snapshot.Add(window, "window");

	// viewports
	snapshot.Add(viewport_frame_size, "viewport_frame");
	snapshot.Add(layout_id, "layout_id");
	snapshot.Add(projections, "projections");
	snapshot.Add(viewports, "viewports");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
