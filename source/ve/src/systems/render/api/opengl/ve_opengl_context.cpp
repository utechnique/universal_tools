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
	UiScopeLock ui_lock;
	ut::Optional<ut::Error> apply_error = MakeCurrent();
	if (apply_error)
	{
		throw ut::Error(apply_error.Move());
	}
	glGenFramebuffers(1, &framebuffer.GetGlHandle());
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.GetGlHandle());
}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&& other) noexcept = default;

// Move operator.
PlatformContext& PlatformContext::operator =(PlatformContext&& other) noexcept = default;


//----------------------------------------------------------------------------//
// Constructor.
Context::Context(PlatformContext platform_context) : PlatformContext(ut::Move(platform_context))
{}

// Set all the elements in a render target to one value.
void Context::ClearTarget(Target& target, float* color)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.GetGlHandle());
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target.buffer.GetGlHandle(), 0);

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
	ut::Optional<ut::Error> make_current_error = MakeCurrent(display.window);
	if (make_current_error)
	{
		throw ut::Error(make_current_error.Move());
	}

	// enable srgb
	glEnable(GL_FRAMEBUFFER_SRGB);

	// set backbuffer as a destination and texture of the
	// render target as a source
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_FRONT);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.GetGlHandle());
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, display.target.buffer.GetGlHandle(), 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	// copy buffer
	glBlitFramebuffer(0, 0, display.GetWidth(), display.GetHeight(),
	                  0, 0, display.GetWidth(), display.GetHeight(),
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// swap back and front buffers and draw
	display.window.SwapBuffer(vsync);

	// set context back to windowless mode
	make_current_error = MakeCurrent();
	if (make_current_error)
	{
		throw ut::Error(make_current_error.Move());
	}

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
