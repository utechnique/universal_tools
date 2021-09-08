//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
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
class Window : public Fl_Window
{
	// Widget class for caption buttons (minimize, maximize, close).
	class CaptionButtonBox : public Fl_Box
	{
	public:
		CaptionButtonBox(ut::uint32 x,
		                 ut::uint32 y,
		                 ut::uint32 w,
		                 ut::uint32 h,
		                 Fl_Color bkg_color,
		                 Fl_Color push_color,
		                 Fl_Color hover_color,
		                 ut::Function<void()> callback);

		int handle(int e) override;

	private:
		Fl_Color bkg_color;
		Fl_Color push_color;
		Fl_Color hover_color;
		ut::Function<void()> callback;
	};

public:
	// All possible button types that can be present in the caption.
	enum CaptionButtonType
	{
		button_close,
		button_maximize,
		button_restore,
		button_minimize,
		button_count
	};

	// Caption button general widget.
	struct CaptionButton
	{
		ut::Array< ut::Color<4, ut::byte> > icon_data;
		ut::UniquePtr<Fl_RGB_Image> icon;
		ut::UniquePtr<CaptionButtonBox> box;
	};

	// Window can be created with a combination of following flags:
	typedef ut::uint32 StyleFlagsType;
	typedef enum StyleFlagBits : StyleFlagsType
	{
		has_close_button = 0x1,
		has_minimize_button = 0x2,
		has_maximize_button = 0x4,
	} StyleFlagBits;
	typedef StyleFlagsType StyleFlags;

	// Default flags for a common window.
	static const StyleFlags default_flags;

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
	       const char* title = "",
	       ut::uint32 border_size = 1,
	       ut::uint32 caption_height = 24,
	       const Theme& theme = Theme(),
	       StyleFlags style_flags = default_flags);

	// Copying/moving is prohibited.
	Window(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator = (const Window&) = delete;
	Window& operator = (Window&&) = delete;

	// Handles resizing, dragging, focus events, etc.
	virtual int handle(int e) override;

	// Sets the allowable range the user can resize this window to.
	void size_range(int minw, int minh, int maxw = 0, int maxh = 0);

	// Maximizes this window to the extents of the working screen.
	void Maximize();

	// Restores this window back from the maximized (fullscreen) mode.
	void Restore();

	// Minimizes this window.
	void Minimize();

	// Returns a reference to the client area widget.
	Fl_Double_Window& GetClientWindow();

private:
	// Processes resizing events (like dragging the border).
	void ProcessResize();

	// Changes cursor icon according to its position.
	void DetectResizeArea();

	// Creates desired caption button.
	CaptionButton CreateButton(ut::uint32 x,
	                           ut::uint32 y,
	                           ut::uint32 size,
	                           CaptionButtonType type,
	                           ut::Function<void()> callback);

	// Possible editing modes.
	enum Mode
	{
		mode_none,
		mode_resize,
		mode_move
	};

	// Possible resize directions.
	enum ResizeDirection
	{
		resize_inactive,
		resize_left_up,
		resize_up,
		resize_right_up,
		resize_right,
		resize_right_bottom,
		resize_bottom,
		resize_left_bottom,
		resize_left
	};

	// Combination of biwise flags.
	StyleFlags flags;

	// Color theme.
	Theme theme;

	// Current editing mode.
	Mode mode = mode_none;

	// Contains everything except the border.
	ut::UniquePtr<Fl_Group> container;

	// Outer border frame.
	ut::UniquePtr<Fl_Box> border_box;

	// Caption bar.
	ut::UniquePtr<Fl_Window> caption;

	// Title of the window that is shown in the upper
	// left corner of the caption.
	ut::UniquePtr<Fl_Box> caption_text;

	// A set of buttons in the right corner of the caption.
	ut::UniquePtr<Fl_Group> button_group;

	// Caption buttons (minimize, maximize, close).
	CaptionButton buttons[button_count];

	// Area available for the user. One can use client_area->begin()/end()
	// to create child widgets.
	ut::UniquePtr<Fl_Double_Window> client_area;

	// Defines the allowable range the user can resize this window to.
	ut::Rect<int> size_range_rect = ut::Rect<int>(240, 240, 0, 0);

	// Cursor position before the dragging began.
	ut::Vector<2, int> prev_cursor;

    // Window position and size before the dragging began.
	ut::Rect<int> prev_rect;

	// Thickness of the border in pixels.
	int border_width;

	// Current resize direction.
	ResizeDirection resize_direction = resize_inactive;

	// Indicates if this window is in the fullscreen mode at the moment.
	bool fullscreen_mode = false;

	// Window metrics before transition to the fullscreen mode.
	ut::Rect<int> restore_rect;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//