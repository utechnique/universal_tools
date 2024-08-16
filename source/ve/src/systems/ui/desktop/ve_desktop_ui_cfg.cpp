//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
#include "systems/ui/ve_ui_viewport.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
template<> const char* ve::Config<ve::ui::Settings>::skName = "ui";
template<> const char* ve::Config<ve::ui::Theme>::skName = "theme";
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor, creates a theme with the provided color scheme.
Theme::Theme(ColorScheme scheme)
{
	switch (scheme)
	{
	case scheme_dark:
		background_color = ut::Color<3, ut::byte>(51, 51, 55);
		foreground_color = ut::Color<3, ut::byte>(150, 150, 165);
		frame_color = ut::Color<3, ut::byte>(80, 80, 88);
		primary_tab_color = ut::Color<3, ut::byte>(80, 80, 88);
		secondary_tab_color = ut::Color<3, ut::byte>(65, 65, 76);
		button_hover_color = ut::Color<3, ut::byte>(66, 66, 69);
		button_push_color = ut::Color<3, ut::byte>(0, 122, 204);
		viewport_hover_color = ut::Color<3, ut::byte>(115, 115, 120);
		viewport_focus_color = ut::Color<3, ut::byte>(192, 175, 23);
		window_caption_color = ut::Color<3, ut::byte>(51, 51, 55);
		caption_text_color = ut::Color<3, ut::byte>(100, 100, 110);
		caption_icon_color = ut::Color<3, ut::byte>(240, 240, 243);
		focus_border_color = ut::Color<3, ut::byte>(0, 122, 204);
		unfocus_border_color = ut::Color<3, ut::byte>(66, 66, 69);
		break;
	}
	
}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Theme::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(background_color, "background_color");
	snapshot.Add(foreground_color, "foreground_color");
	snapshot.Add(frame_color, "frame_color");
	snapshot.Add(primary_tab_color, "primary_tab_color");
	snapshot.Add(secondary_tab_color, "secondary_tab_color");
	snapshot.Add(button_hover_color, "button_hover_color");
	snapshot.Add(button_push_color, "button_push_color");
	snapshot.Add(viewport_hover_color, "viewport_hover_color");
	snapshot.Add(viewport_focus_color, "viewport_focus_color");
	snapshot.Add(window_caption_color, "window_caption_color");
	snapshot.Add(caption_text_color, "caption_text_color");
	snapshot.Add(caption_icon_color, "caption_icon_color");
	snapshot.Add(focus_border_color, "focus_border_color");
	snapshot.Add(unfocus_border_color, "unfocus_border_color");
}

//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : window(0, 0, 640, 480)
                     , maximized(false)
                     , viewport_frame_size(4)
                     , layout_id(0)
{
	projections.Add(Viewport::perspective);
	projections.Add(Viewport::orthographic_negative_y);
	projections.Add(Viewport::orthographic_positive_x);
	projections.Add(Viewport::orthographic_positive_z);

	render_modes.Add(Viewport::render_mode_complete);
	render_modes.Add(Viewport::render_mode_diffuse);
	render_modes.Add(Viewport::render_mode_diffuse);
	render_modes.Add(Viewport::render_mode_diffuse);
}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	// main window
	snapshot.Add(window, "window");
	snapshot.Add(maximized, "maximized");

	// viewports
	snapshot.Add(viewport_frame_size, "viewport_frame");
	snapshot.Add(layout_id, "layout_id");
	snapshot.Add(projections, "projections");
	snapshot.Add(viewports, "viewports");
	snapshot.Add(render_modes, "render_modes");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
