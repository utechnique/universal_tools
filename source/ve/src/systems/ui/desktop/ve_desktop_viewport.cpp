//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor.
//    @param viewport_id - id associated with this viewport.
//    @param viewport_name - name of the viewport.
//    @param x - the initial horizontal position of the window in pixels.
//    @param y - the initial vertical position of the window in pixels.
//    @param w - the initial width of the window in pixels.
//    @param h - the initial height of the window in pixels.
DesktopViewport::DesktopViewport(Id viewport_id,
                                 ut::String viewport_name,
                                 int x, int y,
                                 int w, int h) : Viewport(viewport_id, ut::Move(viewport_name))
                                               , Fl_Window(x, y, w, h)
{
	UpdateSize(w, h);
}

// Destructors, close signal is triggered here.
DesktopViewport::~DesktopViewport()
{
	CloseSignal();
}

// Returns relative mouse position inside this viewport
// or nothing if it's outside.
ut::Optional< ut::Vector<2> > DesktopViewport::GetCursorPosition()
{
	return cur_mouse_position.Get();
}

// Returns relative mouse position offset from the previous
// frame or nothing if it's outside.
//    @param reset - boolean indicating if current offset must be reset.
ut::Optional< ut::Vector<2> > DesktopViewport::GetCursorOffset(bool reset)
{
	const ut::Optional< ut::Vector<2> > previous = prev_mouse_position.Get();
	const ut::Optional< ut::Vector<2> > current = cur_mouse_position.Get();

	if (reset)
	{
		prev_mouse_position.Set(current);
	}

	if (current && previous)
	{
		return current.Get() - previous.Get();
	}

	return ut::Optional< ut::Vector<2> >();
}

// Assigns new relative mouse position for the current frame.
void DesktopViewport::SetMousePosition(ut::Optional< ut::Vector<2> > position)
{
	cur_mouse_position.Set(position);
}

// Resizes UI widget if resizing is pending.
void DesktopViewport::ResizeCanvas()
{
	if (resize_task)
	{
		Fl_Window::resize(resize_task->offset.X(),
		                  resize_task->offset.Y(),
		                  resize_task->extent.X(),
		                  resize_task->extent.Y());
		resize_task = ut::Optional< ut::Rect<int> >();
		UpdateSize(resize_task->extent.X(), resize_task->extent.Y());
	}
}

// Updates width and height of the current mode.
void DesktopViewport::UpdateSize(int w, int h)
{
	Viewport::Mode new_mode = mode.Get();
	new_mode.width = static_cast<ut::uint32>(w);
	new_mode.height = static_cast<ut::uint32>(h);
	mode.Set(new_mode);
}

// Forces viewport to make closure signal.
void DesktopViewport::CloseSignal()
{
	close_signal(id);
}

// Overriden virtual function of the base class (Fl_Window).
// Resize signal is triggered here to delegate resizing to the renderer.
void DesktopViewport::resize(int x, int y, int w, int h)
{
	UpdateSize(w, h);

	// mark this viewport as needed to be resized
	resize_task = ut::Rect<int>(x, y, w, h);

	// call resize signal, note that some slots can call
	// ResizeCanvas() function internally
	resize_signal(id, w, h);

	// resize ui area
	ResizeCanvas();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
