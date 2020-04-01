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
// Enumeration of all possible OpenGL resources.
namespace gl
{
	enum Type
	{
		texture,
		framebuffer
	};
}

//----------------------------------------------------------------------------//
// Use specialization ve::render::GlDetail template to define a type of
// the OpenGL resource and a way to delete it.
template<gl::Type type> class GlDetail;

//----------------------------------------------------------------------------//
// Texture.
template<> struct GlDetail<gl::texture>
{
	typedef GLuint Handle;
	static void Destroy(GLuint* rc) { glDeleteTextures(1, rc); };
};

// Framebuffer.
template<> struct GlDetail<gl::framebuffer>
{
	typedef GLuint Handle;
	static void Destroy(GLuint* rc) { glDeleteFramebuffers(1, rc); };
};

//----------------------------------------------------------------------------//
// Helper wrapper for OpenGL resources.
template<gl::Type type>
class GlRc
{
public:
	// Pure OpenGL type of the managed resource.
	typedef typename GlDetail<type>::Handle Handle;

	// Constructor
	GlRc(Handle in_rc = 0) : rc(in_rc)
	{}

	// Move constructor.
	GlRc(GlRc&& other) noexcept : rc(other.rc)
	{
		other.rc = 0;
	}

	// Move operator
	GlRc& operator =(GlRc&& other) noexcept
	{
		Delete();
		rc = other.rc;
		other.rc = 0;
		return *this;
	}

	// Assignment operator
	GlRc& operator =(Handle in_rc) noexcept
	{
		Delete();
		rc = in_rc;
	}

	// Copying is prohibited.
	GlRc(const GlRc&) = delete;
	GlRc& operator =(const GlRc&) = delete;

	// Destroyes managed object.
	void Delete()
	{
		if (rc)
		{
			GlDetail<type>::Destroy(&rc);
			rc = 0;
		}
	}

	// Returns pure OpenGL handle of the managed resource.
	Handle& GetGlHandle()
	{
		return rc;
	}

	// Returns pure OpenGL handle of the managed resource.
	const Handle& GetGlHandle() const
	{
		return rc;
	}

private:
	Handle rc;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//