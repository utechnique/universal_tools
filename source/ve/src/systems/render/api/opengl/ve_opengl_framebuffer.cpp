//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_framebuffer.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformFramebuffer::PlatformFramebuffer(GlRc<gl::framebuffer>::Handle gl_framebuffer_id) : GlRc<gl::framebuffer>(gl_framebuffer_id)
{}

// Move constructor.
PlatformFramebuffer::PlatformFramebuffer(PlatformFramebuffer&&) noexcept = default;

// Move operator.
PlatformFramebuffer& PlatformFramebuffer::operator =(PlatformFramebuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//