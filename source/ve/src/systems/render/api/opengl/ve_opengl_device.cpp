//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_device.h"
#include <FL/x.h> // to get viewport window descriptor
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Creates main context for the current thread and initializes OpenGL functions.
Context InitializeGLAndCreateContext()
{
	// load opengl functions
	ut::Optional<ut::Error> init_error = InitOpenGLPlatform();
	if (init_error)
	{
		throw ut::Error(init_error.Move());
	}

	// create platform-specific opengl context
	ut::Result<OpenGLContext, ut::Error> context_result = CreateOpenGLContext();
	if (!context_result)
	{
		throw ut::Error(context_result.MoveAlt());
	}

	// success
	return Context(PlatformContext(context_result.MoveResult()));
}

//----------------------------------------------------------------------------//
// Constructor.
Device::Device() : context(InitializeGLAndCreateContext())
{}

// Move constructor.
Device::Device(Device&&) noexcept = default;

// Move operator.
Device& Device::operator =(Device&&) noexcept = default;

// Creates new texture.
//    @param width - width of the texture in pixels.
//    @param height - height of the texture in pixels.
//    @return - new texture object of error if failed.
ut::Result<Texture, ut::Error> Device::CreateTexture(pixel::Format format, ut::uint32 width, ut::uint32 height)
{
	const GLenum gl_pixel_format = ConvertPixelFormatToOpenGL(format);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, gl_pixel_format, width, height, 0, gl_pixel_format, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return Texture(texture, format);
}

// OpenGL doesn't support deferred contexts.
ut::Result<Context, ut::Error> Device::CreateDeferredContext()
{
	return ut::MakeError(ut::error::not_supported);
}

// Creates platform-specific representation of the rendering area inside a UI viewport.
//    @param viewport - reference to UI viewport containing rendering area.
//    @return - new display object or error if failed.
ut::Result<Display, ut::Error> Device::CreateDisplay(ui::Viewport& viewport)
{
	// extract windows handle from the viewport widget
	const HWND hwnd = fl_xid(&viewport);

	// get size of the viewport
	const ut::uint32 width = static_cast<ut::uint32>(viewport.w());
	const ut::uint32 height = static_cast<ut::uint32>(viewport.h());

	// create texture (backbuffer) for the render target
	ut::Result<Texture, ut::Error> texture = CreateTexture(pixel::r8g8b8a8, width, height);
	if (!texture)
	{
		return ut::MakeError(texture.MoveAlt());
	}

	// create render target that will be associated with provided viewport
	Target target(PlatformRenderTarget(), texture.MoveResult());

	// success
	return Display(PlatformDisplay(OpenGLWindow(hwnd)), ut::Move(target), width, height);
}

// Resizes buffers associated with rendering area inside a UI viewport.
//    @param display - reference to display object.
//    @param width - new width of the display in pixels.
//    @param width - new height of the display in pixels.
//    @return - optional ut::Error if failed.
ut::Optional<ut::Error> Device::ResizeDisplay(Display& display,
                                              ut::uint32 width,
                                              ut::uint32 height)
{
	// resize texture associated with display's render target
	glBindTexture(GL_TEXTURE_2D, display.target.buffer.gl_tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// update display size
	display.width = width;
	display.height = height;

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//