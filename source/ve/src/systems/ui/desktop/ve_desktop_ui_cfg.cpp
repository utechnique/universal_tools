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
	case ColorScheme::dark:
		background_color = ut::Color<3, ut::byte>(51, 51, 55);
		foreground_color = ut::Color<3, ut::byte>(150, 150, 165);
		frame_color = ut::Color<3, ut::byte>(80, 80, 88);
		primary_tab_color = ut::Color<3, ut::byte>(80, 80, 88);
		secondary_tab_color = ut::Color<3, ut::byte>(65, 65, 76);
		button_hover_color = ut::Color<3, ut::byte>(66, 66, 69);
		button_push_color = ut::Color<3, ut::byte>(0, 122, 204);
		input_color = ut::Color<3, ut::byte>(39, 39, 39);
		viewport_hover_color = ut::Color<3, ut::byte>(115, 115, 120);
		viewport_focus_color = ut::Color<3, ut::byte>(192, 175, 23);
		break;
	case ColorScheme::light:
		background_color = ut::Color<3, ut::byte>(238, 238, 242);
		foreground_color = ut::Color<3, ut::byte>(52, 52, 52);
		frame_color = ut::Color<3, ut::byte>(228, 228, 232);
		primary_tab_color = ut::Color<3, ut::byte>(209, 208, 218);
		secondary_tab_color = ut::Color<3, ut::byte>(221, 221, 228);
		button_hover_color = ut::Color<3, ut::byte>(201, 222, 245);
		button_push_color = ut::Color<3, ut::byte>(140, 181, 224);
		input_color = ut::Color<3, ut::byte>(255, 255, 255);
		viewport_hover_color = ut::Color<3, ut::byte>(75, 155, 120);
		viewport_focus_color = ut::Color<3, ut::byte>(250, 175, 23);
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
	snapshot.Add(input_color, "input_color");
	snapshot.Add(viewport_hover_color, "viewport_hover_color");
	snapshot.Add(viewport_focus_color, "viewport_focus_color");
}

// Replaces original FLTK color scheme with own colors.
void Theme::ApplyToFltk() const
{
	// set scheme
	Fl::scheme("base");

	// set colors
	Fl::background(background_color.R(),
	               background_color.G(),
	               background_color.B());

	Fl::background2(background_color.R(),
	                background_color.G(),
	                background_color.B());

	Fl::foreground(foreground_color.R(),
	               foreground_color.G(),
	               foreground_color.B());

	Fl::set_color(FL_LIGHT3,
	              static_cast<ut::byte>(foreground_color.R() * 0.7f),
	              static_cast<ut::byte>(foreground_color.G() * 0.7f),
	              static_cast<ut::byte>(foreground_color.B() * 0.7f));

	// tab
	Fl::set_color(55, primary_tab_color.R(), primary_tab_color.G(), primary_tab_color.B());

	// frame
	Fl::set_color(32, background_color.R(), background_color.G(), background_color.B());
	Fl::set_color(42, frame_color.R(), frame_color.G(), frame_color.B());
	Fl::set_color(44, frame_color.R(), frame_color.G(), frame_color.B());
	Fl::set_color(51, background_color.R(), background_color.G(), background_color.B());
	Fl::set_color(54, frame_color.R(), frame_color.G(), frame_color.B());

	// select from combo box color
	Fl::set_color(15, frame_color.R(), frame_color.G(), frame_color.B());
}

//----------------------------------------------------------------------------//
// Constructor, initializes hotkey bindings.
HotkeyBindings::HotkeyBindings() : show_properties("p")
{}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void HotkeyBindings::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(show_properties, "show_properties");
}

//----------------------------------------------------------------------------//
// Constructor, default values are set here.
Settings::Settings() : window(0, 0, 640, 480)
                     , maximized(false)
                     , viewport_frame_size(4)
                     , layout_id(0)
{
	projections.Add(static_cast<ut::uint32>(Viewport::Projection::perspective));
	projections.Add(static_cast<ut::uint32>(Viewport::Projection::orthographic_negative_y));
	projections.Add(static_cast<ut::uint32>(Viewport::Projection::orthographic_positive_x));
	projections.Add(static_cast<ut::uint32>(Viewport::Projection::orthographic_positive_z));

	render_modes.Add(Viewport::RenderMode::complete);
	render_modes.Add(Viewport::RenderMode::diffuse);
	render_modes.Add(Viewport::RenderMode::diffuse);
	render_modes.Add(Viewport::RenderMode::diffuse);
}

// Registers data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Settings::Reflect(ut::meta::Snapshot& snapshot)
{
	// hot keys
	snapshot.Add(hotkeys, "hotkeys");

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
