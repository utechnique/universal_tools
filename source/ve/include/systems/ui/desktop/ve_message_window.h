//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_window.h"
#include "systems/ui/desktop/ve_button.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Window showing custom message to user.
class MessageWindow : public Window
{
public:
	// Constructor, initializes message box and 'ok' button.
	//    @param x - horizontal coordinate of the upper left corner of the window.
	//    @param y - vertical coordinate of the upper left corner of the window.
	//    @param w - width of the window in pixels.
	//    @param h - height of the window in pixels.
	//    @param text - string with the text of the message box.
	//    @param title - string with window title.
	//    @param theme - reference to the color theme of the window.
	MessageWindow(int x,
	              int y,
	              ut::uint32 w,
	              ut::uint32 h,
	              ut::String text,
	              ut::String title = "Message",
	              const Theme& theme = Theme());

	// Height of the window caption in pixels.
	static const ut::uint32 skCapHeight;

	// Width of the 'Ok' button in pixels.
	static const ut::uint32 skOkButtonWidth;

	// Height of the 'Ok' button in pixels.
	static const ut::uint32 skOkButtonHeight;

private:
	ut::UniquePtr<ut::String> message;
	ut::UniquePtr<Fl_Box> message_box;
	ut::UniquePtr<Button> ok_button;
};

// Shows a dialog window with the provided text and the 'Ok' button.
//    @param x_position - horizontal coordinate of the upper left corner of the window.
//    @param y_position - vertical coordinate of the upper left corner of the window.
//    @param width - width of the window in pixels.
//    @param height - height of the window in pixels.
//    @param text - string with the text of the message box.
//    @param title - string with window title.
//    @param theme - reference to the color theme of the window.
void ShowMessageWindow(int x_position,
                       int y_position,
                       ut::uint32 width,
                       ut::uint32 height,
                       ut::String text,
                       ut::String title = "Message",
                       const Theme& theme = Theme());

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//