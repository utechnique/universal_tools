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
// OpenGL context.
class PlatformContext : public OpenGLContext
{
	friend class Device;
public:
	// Constructor.
	PlatformContext(OpenGLContext opengl_context);

	// Move constructor.
	PlatformContext(PlatformContext&&) noexcept;

	// Move operator.
	PlatformContext& operator =(PlatformContext&&) noexcept;

	// Copying is prohibited.
	PlatformContext(const PlatformContext&) = delete;
	PlatformContext& operator =(const PlatformContext&) = delete;

protected:
	// Present request is a delayed presentation task that is executed later during
	// ve::render::Device::Submit() call.
	struct PresentRequest
	{
		GlRc<gl::framebuffer>::Handle framebuffer;
		GLenum attachment_id;
	};

	ut::Map<GlRc<gl::texture>::Handle, PresentRequest> present_queue;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//