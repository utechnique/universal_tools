//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// FL_INTERNALS is needed to acces Fl_X::make() in order to perform Windows
// fix of the windows not showing in the taskbar.
#define FL_INTERNALS
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_window.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_logo.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
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
Window::Window(ut::uint32 position_x,
               ut::uint32 position_y,
               ut::uint32 width,
               ut::uint32 height,
               ut::String title,
               const Theme& in_theme) : Fl_Double_Window(position_x,
                                                         position_y,
                                                         width,
                                                         height)
                                      , theme(in_theme)
                                      , title_text(ut::MakeUnique<ut::String>(ut::Move(title)))
{
	icon_img = ut::MakeUnique<Fl_RGB_Image>(g_ve_logo.pixel_data,
	                                        g_ve_logo.width,
	                                        g_ve_logo.height,
	                                        g_ve_logo.bytes_per_pixel);
	icon(icon_img.Get());
	label(title_text->GetAddress());
}

// Returns a refernece to the ui theme of this window.
const Theme& Window::GetTheme() const
{
	return theme;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
