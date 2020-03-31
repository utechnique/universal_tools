//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_viewport.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#if VE_DX11
#include <FL/Fl_Window.H>
typedef Fl_Window FltkRenderWidget;
#elif VE_OPENGL
#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include "glext.h"
typedef Fl_Gl_Window FltkRenderWidget;
#endif
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::DestopViewport is a viewport for desktop applications.
class DesktopViewport : public Viewport, public FltkRenderWidget
{
public:
	// Every viewport must have unique id.
	typedef ut::uint32 Id;

	// Constructor.
	//    @param viewport_id - id associated with this viewport.
	//    @param viewport_name - name of the viewport.
	//    @param x - the initial horizontal position of the window in pixels.
	//    @param y - the initial vertical position of the window in pixels.
	//    @param w - the initial width of the window in pixels.
	//    @param h - the initial height of the window in pixels.
	DesktopViewport(Id viewport_id, ut::String viewport_name, int x, int y, int w, int h);

	// Destructors, close signal is triggered here.
    ~DesktopViewport();

#if VE_OPENGL
	// Attaches texture buffer from a render display to this viewport.
	//    @param src_id - id of the texture associated with display.
	void AttachDisplayBuffer(GLuint src_id);
#endif

private:
#if VE_OPENGL
	// temporal framebuffer is used to blit texture to user
	GLuint framebuffer;

	// texture buffer that is associated with this viewport
	GLuint src_buffer;

	// true if viewport has a texture attached to it
	ut::Atomic<bool> display_attached;

	// Shows a display texture to user.
	void draw() override;
#endif

	// Overriden virtual function of the base class (Fl_Window).
	// Resize signal is triggered here.
	void resize(int x, int y, int w, int h) override;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
