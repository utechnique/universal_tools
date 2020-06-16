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
// OpenGL image.
class PlatformImage : public GlRc<gl::texture>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformImage(GlRc<gl::texture>::Handle gl_tex_id);

	// Move constructor.
	PlatformImage(PlatformImage&&) noexcept;
	PlatformImage& operator =(PlatformImage&&) noexcept;

	// Copying is prohibited.
	PlatformImage(const PlatformImage&) = delete;
	PlatformImage& operator =(const PlatformImage&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//