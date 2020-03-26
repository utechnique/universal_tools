//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_texture.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformTexture::PlatformTexture(GLuint opengl_texture) : gl_tex_id(opengl_texture)
{}

// Destructor.
PlatformTexture::~PlatformTexture()
{
	Destroy();
}

// Move constructor.
PlatformTexture::PlatformTexture(PlatformTexture&& other) noexcept : gl_tex_id(other.gl_tex_id)
{
	other.gl_tex_id = GL_FALSE;
}

// Move operator.
PlatformTexture& PlatformTexture::operator =(PlatformTexture&& other) noexcept
{
	//Destroy();
	gl_tex_id = other.gl_tex_id;
	other.gl_tex_id = GL_FALSE;
	return *this;
}

// Destroys managed object.
void PlatformTexture::Destroy()
{
	if (gl_tex_id != GL_FALSE)
	{
		glDeleteTextures(1, &gl_tex_id);
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//