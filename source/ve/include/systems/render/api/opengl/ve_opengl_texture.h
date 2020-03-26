//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// OpenGL texture.
class PlatformTexture
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformTexture(GLuint opengl_texture);

	// Destructor.
	~PlatformTexture();

	// Move constructor.
	PlatformTexture(PlatformTexture&&) noexcept;
	PlatformTexture& operator =(PlatformTexture&&) noexcept;

	// Copying is prohibited.
	PlatformTexture(const PlatformTexture&) = delete;
	PlatformTexture& operator =(const PlatformTexture&) = delete;

protected:
	GLuint gl_tex_id;

private:
	// Destroys managed object.
	void Destroy();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//