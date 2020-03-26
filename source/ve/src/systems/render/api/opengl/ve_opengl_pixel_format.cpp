//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_pixel_format.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts pixel format to the one compatible with OpenGL.
GLenum ConvertPixelFormatToOpenGL(pixel::Format format)
{
	switch (format)
	{
	case pixel::r8g8b8a8: return GL_RGBA;
	case pixel::r8g8b8a8_srgb: return GL_RGBA;
	}
	return GL_COLOR_INDEX;
}

// Converts OpenGL pixel format to ve::render::pixel::Format value.
pixel::Format ConvertPixelFormatFromOpenGL(GLenum format)
{
	switch (format)
	{
	case GL_RGBA: return pixel::r8g8b8a8;
	case GL_SRGB_ALPHA: return pixel::r8g8b8a8_srgb;
	}
	return pixel::unknown;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//