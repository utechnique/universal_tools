//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
#include "systems/render/api/opengl/ve_opengl_resource.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Forward declarations.
class Display;
class Context;

//----------------------------------------------------------------------------//
// OpenGL device.
class PlatformDevice
{
public:
	// Constructor
	PlatformDevice();

	// Move constructor.
	PlatformDevice(PlatformDevice&&) noexcept;

	// Move operator.
	PlatformDevice& operator =(PlatformDevice&&) noexcept;

	// Copying is prohibited.
	PlatformDevice(const PlatformDevice&) = delete;
	PlatformDevice& operator =(const PlatformDevice&) = delete;

	// Presents final image to user.
	//    @param window - reference to the destination window.
	//    @param src_framebuffer - framebuffer to copy contents from.
	//    @param src_attachment_id - id of the attachment in @src_framebuffer.
	//    @param width - width of the backbuffer in pixels.
	//    @param height - height of the backbuffer in pixels.
	void Present(OpenGLWindow& window,
	             GlRc<gl::framebuffer>::Handle src_framebuffer,
	             GLenum src_attachment_id,
	             GLint width,
	             GLint height);

protected:
	ut::UniquePtr<Context> context;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
