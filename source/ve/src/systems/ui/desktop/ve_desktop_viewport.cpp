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
{}

// Destructors, close signal is triggered here.
DesktopViewport::~DesktopViewport()
{
	CloseSignal();
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
	}
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
