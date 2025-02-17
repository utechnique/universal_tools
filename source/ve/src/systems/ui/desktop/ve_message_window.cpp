//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_message_window.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Height of the caption in pixels.
const ut::uint32 MessageWindow::skCapHeight = 24;

// Width of the 'Ok' button in pixels.
const ut::uint32 MessageWindow::skOkButtonWidth = 64;

// Height of the 'Ok' button in pixels.
const ut::uint32 MessageWindow::skOkButtonHeight = 32;

//----------------------------------------------------------------------------//
// Constructor, initializes message box and 'ok' button.
//    @param x - horizontal coordinate of the upper left corner of the window.
//    @param y - vertical coordinate of the upper left corner of the window.
//    @param w - width of the window in pixels.
//    @param h - height of the window in pixels.
//    @param text - string with the text of the message box.
//    @param title - string with window title.
//    @param theme - reference to the color theme of the window.
MessageWindow::MessageWindow(int x_position,
                             int y_position,
                             ut::uint32 width,
                             ut::uint32 height,
                             ut::String text,
                             ut::String title,
                             const Theme& theme) : Window(x_position,
                                                          y_position,
                                                          ut::Max<ut::uint32>(width, skOkButtonWidth + 2),
                                                          ut::Max<ut::uint32>(height, skCapHeight + skOkButtonHeight + 2),
                                                          title,
	                                                      theme)
                                                 , message(ut::MakeUnique<ut::String>(text))
{
	message_box = ut::MakeUnique<Fl_Box>(0,
	                                     0,
	                                     w(),
	                                     h() - skOkButtonHeight,
	                                     message->GetAddress());
	message_box->box(FL_NO_BOX);
	message_box->color(ConvertToFlColor(theme.background_color));
	message_box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

	ok_button = ut::MakeUnique<Button>(w() / 2 - skOkButtonWidth / 2,
	                                   h() - skOkButtonHeight,
	                                   skOkButtonWidth,
	                                   skOkButtonHeight,
	                                  "Ok");
	ok_button->SetBackgroundColor(Button::State::release, ConvertToFlColor(theme.background_color));
	ok_button->SetBackgroundColor(Button::State::push, ConvertToFlColor(theme.button_push_color));
	ok_button->SetBackgroundColor(Button::State::hover, ConvertToFlColor(theme.button_hover_color));
	ok_button->SetCallback([&]() { hide(); });

	resizable(nullptr);
	end();
}

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
                       ut::String title,
                       const Theme& theme)
{
	// Fltk 1.3 has the bug when the parentless window automaticly receives
	// a parent if it was created inside Fl_Widget::handle(). Below is the hack
	// to overcome this unwanted behaviour.
	ut::UniquePtr<Fl_Window> test_widget = ut::MakeUnique<Fl_Window>(100000, 100000, 0, 0);
	test_widget->show();
	test_widget->hide();
	test_widget.Delete();

	// Create the window and wait until the user makes a choice.
	ut::UniquePtr<MessageWindow> msg_wnd = ut::MakeUnique<MessageWindow>(x_position,
	                                                                     y_position,
	                                                                     width,
	                                                                     height,
	                                                                     text,
	                                                                     title,
	                                                                     theme);
	msg_wnd->set_modal();
	msg_wnd->show();
	while (msg_wnd->shown())
	{
		Fl::wait();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
