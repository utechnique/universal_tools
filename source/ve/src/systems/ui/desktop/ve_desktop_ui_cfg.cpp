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
// Converts provided color to the FLTK rgb color.
Fl_Color ConvertToFlColor(const ut::Color<3, ut::byte>& color)
{
	return fl_rgb_color(color.R(), color.G(), color.B());
}

//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Theme::Theme() : background_color(51, 51, 55)
               , foreground_color(150, 150, 165)
               , frame_color(80, 80, 88)
               , tab_color(80, 80, 88)
               , viewport_hover_color(115, 115, 120)
               , viewport_focus_color(192, 175, 23)
               , window_caption_color(51, 51, 55)
               , caption_text_color(100, 100, 110)
               , caption_button_color(240, 240, 243)
               , focus_border_color(0, 122, 204)
               , unfocus_border_color(66, 66, 69)
{}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Theme::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(background_color, "background_color");
	snapshot.Add(foreground_color, "foreground_color");
	snapshot.Add(frame_color, "frame_color");
	snapshot.Add(tab_color, "tab_color");
	snapshot.Add(viewport_hover_color, "viewport_hover_color");
	snapshot.Add(viewport_focus_color, "viewport_focus_color");
	snapshot.Add(window_caption_color, "window_caption_color");
	snapshot.Add(caption_text_color, "caption_text_color");
	snapshot.Add(caption_button_color, "caption_button_color");
	snapshot.Add(focus_border_color, "focus_border_color");
	snapshot.Add(unfocus_border_color, "unfocus_border_color");
}

//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : window(0, 0, 640, 480)
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
