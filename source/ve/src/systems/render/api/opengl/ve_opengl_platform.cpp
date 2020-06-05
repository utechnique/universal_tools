//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Returns OpenGl id of the color attachment using the given
// value from 0 to skMaxGlColorAttachments.
GLenum GetColorAttachmentId(ut::uint32 id)
{
	switch (id)
	{
	case 0: return GL_COLOR_ATTACHMENT0;
	case 1: return GL_COLOR_ATTACHMENT1;
	case 2: return GL_COLOR_ATTACHMENT2;
	case 3: return GL_COLOR_ATTACHMENT3;
	case 4: return GL_COLOR_ATTACHMENT4;
	case 5: return GL_COLOR_ATTACHMENT5;
	case 6: return GL_COLOR_ATTACHMENT6;
	case 7: return GL_COLOR_ATTACHMENT7;
	case 8: return GL_COLOR_ATTACHMENT8;
	case 9: return GL_COLOR_ATTACHMENT9;
	case 10: return GL_COLOR_ATTACHMENT10;
	case 11: return GL_COLOR_ATTACHMENT11;
	case 12: return GL_COLOR_ATTACHMENT12;
	case 13: return GL_COLOR_ATTACHMENT13;
	case 14: return GL_COLOR_ATTACHMENT14;
	case 15: return GL_COLOR_ATTACHMENT15;
	}
	throw ut::Error(ut::error::invalid_arg, "OpenGL: invalid color attachment id.");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL && UT_WINDOWS
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
