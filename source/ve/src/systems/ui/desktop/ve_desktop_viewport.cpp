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
                                               , FltkRenderWidget(x, y, w, h)
#if VE_OPENGL
                                               , framebuffer(0)
                                               , src_buffer(0)
                                               , display_attached(false)
#endif
{
#if VE_OPENGL
	mode(FL_RGB | FL_OPENGL3);
#endif
}

// Destructors, close signal is triggered here.
DesktopViewport::~DesktopViewport()
{
    close_signal(id);
#if VE_OPENGL
	if (framebuffer)
	{
		render::glDeleteFramebuffers(1, &framebuffer);
	}
#endif
}

// Overriden virtual function of the base class (Fl_Window).
// Resize signal is triggered here.
void DesktopViewport::resize(int x, int y, int w, int h)
{
	FltkRenderWidget::resize(x, y, w, h);
	resize_signal(id, w, h);
}

//----------------------------------------------------------------------------//
#if VE_OPENGL
// Attaches texture buffer from a render display to this viewport.
//    @param src_id - id of the texture associated with display.
void DesktopViewport::AttachDisplayBuffer(GLuint src_id)
{
	if (!display_attached.Read())
	{
		render::glGenFramebuffers(1, &framebuffer);
	}

	src_buffer = src_id;
	display_attached.Store(true);
}

// Shows a display texture to user.
void DesktopViewport::draw()
{
	// exit if there is nothing to draw
	if (!display_attached.Read())
	{
		return;
	}

	// enable srgb
	glEnable(GL_FRAMEBUFFER_SRGB);

	// set backbuffer as a destination and a display texture as a source
	render::glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	render::glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	render::glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, src_buffer, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// copy buffer
	render::glBlitFramebuffer(0, 0, w(), h(),
	                          0, 0, w(), h(),
	                          GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// test code
	/*
	static int it = 0;
	it++;
	bool swap_col = it % 1000 < 500;
	float color[4] = { swap_col ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f };
	glClearColor(color[0], color[1], color[2], 0);
	glClear(GL_COLOR_BUFFER_BIT);
	*/
}

#endif // VE_OPENGL
//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
