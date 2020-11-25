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
                         const Framebuffer::Info& framebuffer_info,
                         ut::Array<Target::SharedData> in_color_targets,
                         ut::Optional<Target::SharedData> in_depth_stencil_target) : PlatformFramebuffer(ut::Move(platform_framebuffer))
                                                                                   , info(framebuffer_info)
                                                                                   , color_targets(ut::Move(in_color_targets))
                                                                                   , depth_stencil_target(ut::Move(in_depth_stencil_target))
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