//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_choice_window.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Height of the caption in pixels.
const ut::uint32 ChoiceWindow::skCapHeight = 24;

// Default width of the choice window in pixels.
const ut::uint32 ChoiceWindow::skDefaultWidth = 280;

// Default height of the choice window in pixels.
const ut::uint32 ChoiceWindow::skDefaultHeight = 480;

//----------------------------------------------------------------------------//
// Constructor. Creates a set of buttons for each variant.
//    @param x - horizontal coordinate of the upper left corner of the window.
//    @param y - vertical coordinate of the upper left corner of the window.
//    @param w - width of the window in pixels.
//    @param h - height of the window in pixels.
//    @param title - string with window title.
//    @param variants - const reference to the array of strings, each string
//                      represents a variant that can be chosen.
//    @param theme - reference to the color theme of the window.
ChoiceWindow::ChoiceWindow(int x_position,
                           int y_position,
                           ut::uint32 width,
                           ut::uint32 height,
                           ut::String title,
                           const ut::Array<ut::String>& variants,
                           const Theme& theme) : Window(x_position,
                                                        y_position,
                                                        width,
                                                        height,
                                                        ut::Move(title),
                                                        1, skCapHeight,
	                                                    theme,
                                                        Window::has_close_button)
{
	const int variant_height = 32;
	const int margin = 24;

	Fl_Double_Window& client_area = GetClientWindow();
	client_area.begin();

	// user must be able to scroll down if the number of variants is big
	scroll = ut::MakeUnique<Scroll>(0, 0, client_area.w(), client_area.h());
	scroll->type(Fl_Scroll::VERTICAL);
	scroll->scrollbar_size(Fl::scrollbar_size());
	scroll->resizable(scroll.Get());

	// create a button for each variant
	const ut::uint32 variant_count = static_cast<ut::uint32>(variants.Count());
	variant_id.Resize(variant_count);
	for (ut::uint32 i = 0; i < variant_count; i++)
	{
		const int x_offset = margin;
		const int y_offset = static_cast<int>(i) * variant_height;

		VariantId& variant = variant_id[i];
		variant.value = i;

		// create button
		ut::UniquePtr<Button> variant_button = ut::MakeUnique<Button>(x_offset,
		                                                              y_offset,
		                                                              width - margin * 2,
		                                                              variant_height,
		                                                              variants[i]);
		variant_button->SetBackgroundColor(Button::state_release, ConvertToFlColor(theme.window_caption_color));
		variant_button->SetBackgroundColor(Button::state_push, ConvertToFlColor(theme.focus_border_color));
		variant_button->SetBackgroundColor(Button::state_hover, ConvertToFlColor(theme.unfocus_border_color));
		variant_button->SetCallback([&]() { result = variant.Read(); hide(); });
		variant_buttons.Add(ut::Move(variant_button));
	}

	// finish groups
	scroll->end();
	client_area.resizable(scroll.Get());
	client_area.end();

	// crop window height if it's too big
	const int variant_total_height = variant_height * variant_count;
	const int height_diff = variant_total_height - client_area.h();
	if (height_diff < 0)
	{
		resize(x_position, y_position, width, height + height_diff);
	}
}

// Returns the index of the selected variant or nothing if canceled.
ut::Optional<ut::uint32> ChoiceWindow::GetResult() const
{
	return result;
}

//----------------------------------------------------------------------------//
// Shows a modal (blocking) window with the set of variants. User must choose one
// of them.
//    @param x_position - horizontal coordinate of the upper left corner of the window.
//    @param y_position - vertical coordinate of the upper left corner of the window.
//    @param width - width of the window in pixels.
//    @param height - height of the window in pixels.
//    @param title - string with window title.
//    @param variants - const reference to the array of strings, each string
//                      represents a variant that can be chosen.
//    @param theme - reference to the color theme of the window.
//    @return - id of the chosen variant or nothing if canceled.
ut::Optional<ut::uint32> SelectInDialogWindow(int x_position,
                                              int y_position,
                                              ut::uint32 width,
                                              ut::uint32 height,
                                              ut::String title,
                                              const ut::Array<ut::String>& variants,
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
	ut::UniquePtr<ChoiceWindow> choice_wnd = ut::MakeUnique<ChoiceWindow>(x_position,
	                                                                      y_position,
	                                                                      width,
	                                                                      height,
	                                                                      ut::Move(title),
	                                                                      variants,
	                                                                      theme);
	choice_wnd->set_modal();
	choice_wnd->show();
	while (choice_wnd->shown())
	{
		Fl::wait();
	}

	return choice_wnd->GetResult();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
