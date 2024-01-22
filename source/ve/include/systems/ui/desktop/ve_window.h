//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
#include "systems/ui/desktop/ve_button.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Fancy window.
class Window : public Fl_Double_Window
{
public:
	// Constructor.
	//    @param position_x - x position of the upper left corner.
	//    @param position_y - y position of the upper left corner.
	//    @param width - width of the window in pixels.
	//    @param height - height of the window in pixels.
	//    @param title - text in the caption.
	//    @param border_size - thickness of the outer border in pixels.
	//    @param caption_height - height of the caption in pixels.
	//    @param theme - color theme.
	//    @param style_flags - combination of style flags.
	Window(ut::uint32 position_x,
	       ut::uint32 position_y,
	       ut::uint32 width,
	       ut::uint32 height,
	       ut::String title = ut::String(),
           const Theme& theme = Theme());

	// Copying/moving is prohibited.
	Window(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator = (const Window&) = delete;
	Window& operator = (Window&&) = delete;

	// Virtual destructor for the polymorphic type.
	virtual ~Window() override = default;

	// Returns a refernece to the ui theme of this window.
	const Theme& GetTheme() const;

private:
	// Color theme.
	Theme theme;

	// Text of the title.
	ut::UniquePtr<ut::String> title_text;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//