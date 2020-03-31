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
PlatformContext::PlatformContext(OpenGLDummyWindow::UniquePtr window) : dummy_window(ut::Move(window))
{
	ut::Optional<ut::Error> apply_error = dummy_window->ApplyContext();
	if (apply_error)
	{
		throw ut::Error(apply_error.Move());
	}
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

// Destructor.
PlatformContext::~PlatformContext()
{
	Destroy();
}

// Move constructor.
PlatformContext::PlatformContext(PlatformContext&& other) noexcept : dummy_window(ut::Move(other.dummy_window))
                                                                   , framebuffer(other.framebuffer)
{
	other.framebuffer = GL_FALSE;
}

// Move operator.
PlatformContext& PlatformContext::operator =(PlatformContext&& other) noexcept
{
	//Destroy();
	dummy_window = ut::Move(other.dummy_window);
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

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
