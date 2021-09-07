//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
// FL_INTERNALS is needed to acces Fl_X::make() in order to perform Windows
// fix of the windows not showing in the taskbar.
#if UT_WINDOWS
#define FL_INTERNALS
#endif
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_window.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
const Window::StyleFlags Window::default_flags = Window::has_minimize_button |
                                                 Window::has_maximize_button |
                                                 Window::has_close_button;

//----------------------------------------------------------------------------//
// This function fixes unwanted behaviour of the Fl_Window after calling
// Fl_Window::border(0) - such a window is invisible in the Windows OS taskbar.
#if UT_WINDOWS
void ShowInTaskbar(Fl_Window* w)
{
	HWND wnd = fl_xid(w);
	if (!wnd)
	{
		Fl_Group *grp = Fl_Group::current();
		Fl_X::make(w);
		Fl_Group::current(grp);
		wnd = fl_xid(w);
	}

	LONG_PTR styleEx = GetWindowLongPtr(wnd, GWL_EXSTYLE);
	styleEx &= ~WS_EX_TOOLWINDOW;
	SetWindowLongPtr(wnd, GWL_EXSTYLE, styleEx);

	LONG_PTR style = GetWindowLongPtr(wnd, GWL_STYLE);
	style |= WS_SYSMENU;
	style |= WS_MINIMIZEBOX;
	SetWindowLongPtr(wnd, GWL_STYLE, style);
}
#endif

//----------------------------------------------------------------------------//
// Caption button (minimize, maximize, close) constructor.
Window::CaptionButtonBox::CaptionButtonBox(ut::uint32 x,
                                           ut::uint32 y,
                                           ut::uint32 w,
                                           ut::uint32 h,
                                           Fl_Color in_bkg_color,
                                           Fl_Color in_push_color,
                                           Fl_Color in_hover_color,
                                           ut::Function<void()> in_callback) : Fl_Box(x, y, w, h)
                                                                             , bkg_color(in_bkg_color)
                                                                             , push_color(in_push_color)
                                                                             , hover_color(in_hover_color)
                                                                             , callback(ut::Move(in_callback))
{
	color(bkg_color);
}

// Hover color, push down color and callback is handled here.
int Window::CaptionButtonBox::handle(int e)
{
	int ret = Fl_Box::handle(e);
	const int ex = Fl::event_x();
	const int ey = Fl::event_y();

	switch (e)
	{
	case FL_PUSH:
		color(push_color);
		redraw();
		return 1;
	case FL_RELEASE:
		color(bkg_color);
		redraw();
		if (ex > x() && ey > y() && ex < x() + w() && ey < y() + h())
		{
			callback();
		}
		return 1;
	case FL_MOVE:
		color(hover_color);
		redraw();
		return 1;
	case FL_ENTER:
		color(hover_color);
		redraw();
		return 1;
	case FL_LEAVE:
		color(bkg_color);
		redraw();
		return 1;
	case FL_SHOW:
		color(bkg_color);
		redraw();
		return 1;
	}
	
	return ret;
}

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
               const char* title,
               ut::uint32 border_size,
               ut::uint32 caption_height,
               const Theme& in_theme,
               StyleFlags style_flags) : Fl_Window(position_x,
                                                   position_y,
                                                   width,
                                                   height,
                                                   title)
                                       , flags(style_flags)
                                       , border_width(static_cast<int>(border_size))
                                       , theme(in_theme)
{
	this->begin();

	// create border
	if (border_width > 0)
	{
		border_box = ut::MakeUnique<Fl_Box>(0, 0, width, height);
		border_box->box(FL_FLAT_BOX);
		border_box->color(ConvertToFlColor(theme.focus_border_color));
	}

	// create a conteiner for all other child widgets
	container = ut::MakeUnique<Fl_Group>(border_size,
	                                     border_size,
	                                     width - border_size * 2,
	                                     height - border_size * 2);

	// create caption
	caption = ut::MakeUnique<Fl_Window>(border_size,
	                                    border_size,
	                                    width - border_size * 2,
	                                    caption_height);
	caption->color(ConvertToFlColor(theme.window_caption_color));

	// create caption buttons
	button_group = ut::MakeUnique<Fl_Group>(width - border_size - caption_height * 3,
	                                        border_size,
	                                        caption_height * 3,
	                                        caption_height);
	buttons[button_close] = CreateButton(width - caption_height - border_size * 2,
	                                     0,
	                                     caption_height,
	                                     button_close,
	                                     [&]() { hide(); });
	buttons[button_maximize] = CreateButton(buttons[button_close].box->x() - caption_height,
	                                        0,
	                                        caption_height,
	                                        button_maximize,
	                                        [&]() { Maximize(); });
	buttons[button_restore] = CreateButton(buttons[button_close].box->x() - caption_height,
	                                       0,
	                                       caption_height,
	                                       button_restore,
	                                       [&]() { Restore(); });
	buttons[button_minimize] = CreateButton(buttons[button_maximize].box->x() - caption_height,
	                                        0,
	                                        caption_height,
	                                        button_minimize,
	                                        [&]() { Minimize(); });
	button_group->end();

	// hide caption buttons according to the window style
	buttons[button_restore].box->hide();
	if (!(flags & has_close_button))
	{
		buttons[button_close].box->hide();
	}
	if (!(flags & has_minimize_button))
	{
		buttons[button_minimize].box->hide();
	}
	if (!(flags & has_maximize_button))
	{
		buttons[button_maximize].box->hide();
	}

	// caption title
	caption_text = ut::MakeUnique<Fl_Box>(0, 0, button_group->x(), caption_height, title);
	caption_text->box(FL_NO_BOX);
	caption_text->color(ConvertToFlColor(theme.window_caption_color));
	caption_text->label(title);
	caption_text->labelsize(caption_height / 3 * 2);
	caption_text->labelcolor(ConvertToFlColor(theme.caption_text_color));
	caption_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	// finish caption widget
	caption->end();
	caption->resizable(caption_text.Get());

	// create user space window
	client_area = ut::MakeUnique<Fl_Double_Window>(border_size,
	                                               border_size + caption_height,
	                                               width - border_size*2,
	                                               height - border_size * 2 - caption_height);
	client_area->show();
	client_area->end();

	// finish creating widgets
	container->end();
	container->resizable(client_area.Get());
	end();
	resizable(container.Get());

	// disable OS frame
	border(0);

	// check if window is in fullscreen mode
	int sx, sy, sw, sh;
	Fl::screen_work_area(sx, sy, sw, sh);
	if (position_x == sx && position_y == sy &&
	    width == sw && height == sh)
	{
		Maximize();
		restore_rect = ut::Rect<int>(sw / 4, sh / 4, sw / 2, sh / 2);
	}

	// show this window in the OS taskbar
#if UT_WINDOWS
	ShowInTaskbar(this);
#endif
}

// Handles resizing, dragging, focus events, etc.
int Window::handle(int e)
{
	int ret = Fl_Window::handle(e);
	switch (e)
	{
		case FL_PUSH:
		{
			Fl::focus(this);

			if (resize_direction != resize_inactive)
			{
				mode = mode_resize;
			}
			else if(!fullscreen_mode && (Fl::event_y() < caption->h()))
			{
				mode = mode_move;
			}
			else
			{
				mode = mode_none;
			}
			
			cursor_offset = ut::Vector<2, ut::int32>(x() - Fl::event_x_root(),
													 y() - Fl::event_y_root());
			return 1;
		}
		case FL_RELEASE:
			mode = mode_none;
			return 1;
		case FL_DRAG:
			if (mode == mode_resize)
			{
				ProcessResize();
			}
			else if(mode == mode_move)
			{
				position(cursor_offset.X() + Fl::event_x_root(),
						 cursor_offset.Y() + Fl::event_y_root());
			}
			return 1;
		case FL_MOVE:
			DetectResizeArea();
			return 1;
		case FL_ENTER:
			DetectResizeArea();
			return 1;
		case FL_LEAVE:
			cursor(FL_CURSOR_DEFAULT);
			return 1;
		case FL_FOCUS:
			if (border_width > 0)
			{
				border_box->color(ConvertToFlColor(theme.focus_border_color));
			}
			redraw();
			return 1;
		case FL_UNFOCUS:
#if UT_WINDOWS
			if(fl_xid(this) != GetForegroundWindow())
#endif
			{
				if (border_width > 0)
				{
					border_box->color(ConvertToFlColor(theme.unfocus_border_color));
				}
				redraw();
			}
			return 1;
	}

	return ret;
}

// Sets the allowable range the user can resize this window to.
void Window::size_range(int minw, int minh, int maxw, int maxh)
{
	size_range_rect = ut::Rect<int>(minw, minh, maxw, maxh);
	Fl_Window::size_range(minw, minh, maxw, maxh);
}

// Maximizes this window to the extents of the working screen.
void Window::Maximize()
{
	restore_rect = ut::Rect<int>(x(), y(), w(), h());

	if (border_width > 0)
	{
		border_box->hide();
	}

	int sx, sy, sw, sh;
	Fl::screen_work_area(sx, sy, sw, sh);
	resize(sx, sy, sw, sh);

	container->resize(0, 0, sw, sh);

	buttons[button_maximize].box->hide();
	buttons[button_restore].box->show();

	fullscreen_mode = true;

	redraw();
}

// Restores this window back from the maximized (fullscreen) mode.
void Window::Restore()
{
	if (!fullscreen_mode)
	{
		return;
	}

	resize(restore_rect.offset.X(),
	       restore_rect.offset.Y(),
	       restore_rect.extent.X(),
	       restore_rect.extent.Y());
	
	if (border_width > 0)
	{
		border_box->resize(0,
		                   0,
		                   restore_rect.extent.X(),
		                   restore_rect.extent.Y());
		border_box->show();
	}

	container->resize(border_width,
	                  border_width,
	                  restore_rect.extent.X() - border_width * 2,
	                  restore_rect.extent.Y() - border_width * 2);

	buttons[button_restore].box->hide();
	buttons[button_maximize].box->show();

	fullscreen_mode = false;

	redraw();
}

// Minimizes this window.
void Window::Minimize()
{
#if UT_WINDOWS
	ShowWindow(fl_xid(this), SW_MINIMIZE);
#endif
}

// Returns a reference to the client area widget.
Fl_Double_Window& Window::GetClientWindow()
{
	return client_area.GetRef();
}

// Processes resizing events (like dragging the border).
void Window::ProcessResize()
{
	if (fullscreen_mode)
	{
		return;
	}

	ut::Rect<ut::int32> rect(x(), y(), w(), h());
	const ut::Vector<2, ut::int32> cursor(Fl::event_x(), Fl::event_y());
	const ut::Vector<2, ut::int32> offset(rect.extent.X() - cursor.X() > size_range_rect.offset.X() ? cursor.X() : 0,
	                                      rect.extent.Y() - cursor.Y() > size_range_rect.offset.Y() ? cursor.Y() : 0);
	const ut::Vector<2, ut::int32> extension(ut::Max(cursor.X(), size_range_rect.offset.X()),
	                                         ut::Max(cursor.Y(), size_range_rect.offset.Y()));
	switch (resize_direction)
	{
	case resize_left_up:
		rect.offset += offset;
		rect.extent -= offset;
		break;
	case resize_up:
		rect.offset.Y() += offset.Y();
		rect.extent.Y() -= offset.Y();
		break;
	case resize_right_up:
		rect.extent.X() = extension.X();
		rect.offset.Y() += offset.Y();
		rect.extent.Y() -= offset.Y();
		break;
	case resize_right:
		rect.extent.X() = extension.X();
		break;
	case resize_right_bottom:
		rect.extent.X() = extension.X();
		rect.extent.Y() = extension.Y();
		break;
	case resize_bottom:
		rect.extent.Y() = extension.Y();
		break;
	case resize_left_bottom:
		rect.offset.X() += offset.X();
		rect.extent.X() -= offset.X();
		rect.extent.Y() = extension.Y();
		break;
	case resize_left:
		rect.offset.X() += offset.X();
		rect.extent.X() -= offset.X();
		break;
	}

	resize(rect.offset.X(),
	       rect.offset.Y(),
	       rect.extent.X(),
	       rect.extent.Y());

	container->resize(border_width,
	                  border_width,
	                  rect.extent.X() - border_width * 2,
	                  rect.extent.Y() - border_width * 2);

	redraw();
	
	if (border_width > 0)
	{
		border_box->redraw();
	}
}

// Changes cursor icon according to its position.
void Window::DetectResizeArea()
{
	if (fullscreen_mode)
	{
		resize_direction = resize_inactive;
		return;
	}

	const int ax = ut::Abs(Fl::event_x());
	const int ay = ut::Abs(Fl::event_y());

	const int asw = ut::Abs(w() - Fl::event_x() - 1);
	const int ash = ut::Abs(h() - Fl::event_y() - 1);

	if (ax < border_width && ay < border_width)
	{
		resize_direction = resize_left_up;
		cursor(FL_CURSOR_NW);
	}
	else if (asw < border_width && ash < border_width)
	{
		resize_direction = resize_right_bottom;
		cursor(FL_CURSOR_SE);
	}
	else if (ax < border_width && ash < border_width)
	{
		resize_direction = resize_left_bottom;
		cursor(FL_CURSOR_SW);
	}
	else if (asw < border_width && ay < border_width)
	{
		resize_direction = resize_right_up;
		cursor(FL_CURSOR_NE);
	}
	else if (ax < border_width)
	{
		resize_direction = resize_left;
		cursor(FL_CURSOR_W);
	}
	else if (asw < border_width)
	{
		resize_direction = resize_right;
		cursor(FL_CURSOR_E);
	}
	else if (ay < border_width)
	{
		resize_direction = resize_up;
		cursor(FL_CURSOR_N);
	}
	else if (ash < border_width)
	{
		resize_direction = resize_bottom;
		cursor(FL_CURSOR_S);
	}
	else if (resize_direction != resize_inactive)
	{
		cursor(FL_CURSOR_DEFAULT);
		resize_direction = resize_inactive;
	}
}

// Creates desired caption button.
Window::CaptionButton Window::CreateButton(ut::uint32 x,
                                           ut::uint32 y,
                                           ut::uint32 icon_size,
                                           Window::CaptionButtonType type,
                                           ut::Function<void()> callback)
{
	Window::CaptionButton out;

	out.box = ut::MakeUnique<CaptionButtonBox>(x,
	                                           y,
	                                           icon_size,
	                                           icon_size,
	                                           ConvertToFlColor(theme.window_caption_color),
	                                           ConvertToFlColor(theme.focus_border_color),
	                                           ConvertToFlColor(theme.unfocus_border_color),
	                                           ut::Move(callback));
	out.box->box(FL_FLAT_BOX);

	out.icon_data.Resize(icon_size * icon_size);

	const ut::Color<4, ut::byte> color(theme.caption_button_color.R(),
	                                   theme.caption_button_color.G(),
	                                   theme.caption_button_color.B(),
	                                   255);
	const int size = static_cast<int>(icon_size);
	const int margin = size / 3;
	const int odd_fixer = size % 2 == 0 ? 1 : 0;
	const int inv_margin = size - margin - odd_fixer;

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			ut::Color<4, ut::byte>& pixel = out.icon_data[y*size + x];
			pixel = color;
			pixel.A() = 0;

			bool inside = x >= margin && x <= inv_margin && y >= margin && y <= inv_margin;
			if (!inside)
			{
				continue;
			}

			if (type == button_close)
			{
				const int line_width = 1;

				int d0 = ut::Abs(y - x);
				int d1 = ut::Abs(y - size + x + odd_fixer);

				if ((x == margin || x == inv_margin) &&
					(y == margin || y == inv_margin))
				{
					pixel.A() = color.A() - color.A() / 4;
				}
				else if (d0 < line_width || d1 < line_width)
				{
					pixel.A() = color.A();
					
				}
				else if (d0 < line_width + 1 || d1 < line_width + 1)
				{
					pixel.A() = color.A() / 2;
				}
			}
			else if (type == button_maximize)
			{
				const int cap = size / 10;
				const int inner_margin = margin + 1;
				const int inv_inner_margin = inv_margin - 1;
				bool inside_max = x >= inner_margin && x <= inv_inner_margin &&
				                  y >= inner_margin + cap && y <= inv_inner_margin;
				if (!inside_max)
				{
					pixel.A() = color.A() - color.A() / 5;
				}
			}
			else if (type == button_restore)
			{
				ut::Rect<float> rect;

				const int offset = size / 10;
				const int cap = size / 10;
				const int oend = inv_margin - offset;
				const int ostart = margin + offset;

				bool fg_rect = ((x == margin || x == oend) && y >= ostart && y <= inv_margin) ||
				               ((y == ostart || y == inv_margin) && x >= margin && x <= oend);
				bool bg_rect = ((x == ostart && y < ostart) || (x == inv_margin && y >= margin && y <= oend)) ||
				               (((y == margin && x >= ostart && x <= inv_margin) || (y == oend && x > oend)));
				if (fg_rect || bg_rect)
				{
					pixel.A() = color.A() - color.A() / 5;
				}
			}
			else if (type == button_minimize)
			{
				const int dash = size / 8;
				if (y > size - margin - dash - odd_fixer)
				{
					pixel.A() = color.A() - color.A() / 5;
				}
			}
		}
	}

	out.icon = ut::MakeUnique<Fl_RGB_Image>(out.icon_data.GetFirst().GetData(), size, size, 4);

	out.box->image(out.icon.Get());

	return out;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
