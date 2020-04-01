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
PlatformTexture::PlatformTexture(GlRc<gl::texture>::Handle gl_tex_id) : GlRc<gl::texture>(gl_tex_id)
{}

// Move constructor.
PlatformTexture::PlatformTexture(PlatformTexture&& other) noexcept = default;

// Move operator.
PlatformTexture& PlatformTexture::operator =(PlatformTexture&& other) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
