//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_framebuffer.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Framebuffer::Framebuffer(PlatformFramebuffer platform_framebuffer,
                         const FramebufferInfo& framebuffer_info,
                         ut::Array< ut::Ref<Target> > in_color_targets,
                         ut::Optional<Target&> in_ds_target) : PlatformFramebuffer(ut::Move(platform_framebuffer))
                                                             , color_targets(ut::Move(in_color_targets))
                                                             , depth_stencil_target(ut::Move(in_ds_target))
                                                             , info(framebuffer_info)
{}

// Move constructor.
Framebuffer::Framebuffer(Framebuffer&&) noexcept = default;

// Move operator.
Framebuffer& Framebuffer::operator =(Framebuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//