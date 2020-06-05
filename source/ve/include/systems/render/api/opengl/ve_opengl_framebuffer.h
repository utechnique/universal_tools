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
// OpenGL framebuffer.
class PlatformFramebuffer : public GlRc<gl::framebuffer>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformFramebuffer(GlRc<gl::framebuffer>::Handle gl_framebuffer_id);

	// Move constructor.
	PlatformFramebuffer(PlatformFramebuffer&&) noexcept;

	// Move operator.
	PlatformFramebuffer& operator =(PlatformFramebuffer&&) noexcept;

	// Copying is prohibited.
	PlatformFramebuffer(const PlatformFramebuffer&) = delete;
	PlatformFramebuffer& operator =(const PlatformFramebuffer&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//