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
// OpenGL texture.
class PlatformTexture : public GlRc<gl::texture>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformTexture(GlRc<gl::texture>::Handle gl_tex_id);

	// Move constructor.
	PlatformTexture(PlatformTexture&&) noexcept;
	PlatformTexture& operator =(PlatformTexture&&) noexcept;

	// Copying is prohibited.
	PlatformTexture(const PlatformTexture&) = delete;
	PlatformTexture& operator =(const PlatformTexture&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//