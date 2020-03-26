//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_context.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformContext::PlatformContext(OpenGLContext opengl_context) : OpenGLContext(ut::Move(opengl_context))
{
	MakeCurrent();
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

// Destructor.
PlatformContext::~PlatformContext()
{
	Destroy();
}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&& other) noexcept : OpenGLContext(ut::Move(other))
                                                                   , framebuffer(other.framebuffer)
{
	other.framebuffer = GL_FALSE;
}

// Move operator.
PlatformContext& PlatformContext::operator =(PlatformContext&& other) noexcept
{
	//Destroy();
	OpenGLContext::operator=(ut::Move(other));
	framebuffer = other.framebuffer;
	return *this;
}

// Destroys managed object.
void PlatformContext::Destroy()
{
	if (framebuffer != GL_FALSE)
	{
		glDeleteFramebuffers(1, &framebuffer);
	}
}

//----------------------------------------------------------------------------//
// Constructor.
Context::Context(PlatformContext platform_context) : PlatformContext(ut::Move(platform_context))
{}

// Set all the elements in a render target to one value.
void Context::ClearTarget(Target& target, float* color)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target.buffer.gl_tex_id, 0);

	// set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	// set clear color
	glClearColor(color[0], color[1], color[2], color[3]);

	// clear
	glClear(GL_COLOR_BUFFER_BIT);
}

// Presents a rendered image to the user.
//    @param display - reference to the display to show image on.
//    @param vsync - 'true' to enable vertical synchronization.
void Context::Present(Display& display, bool vsync)
{
	// switch context using window's hdc
	MakeCurrent(display.window);

	// enable srgb
	glEnable(GL_FRAMEBUFFER_SRGB);

	// set backbuffer as a destination and texture of the
	// render target as a source
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_FRONT);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, display.target.buffer.gl_tex_id, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// copy buffer
	glBlitFramebuffer(0, 0, display.GetWidth(), display.GetHeight(),
	                  0, 0, display.GetWidth(), display.GetHeight(),
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// swap back and front buffers and draw
	if (vsync)
	{
		SwapBuffers(display.window.context);
	}

	// set context back to windowless mode
	MakeCurrent();

	// disable srgb
	glDisable(GL_FRAMEBUFFER_SRGB);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//