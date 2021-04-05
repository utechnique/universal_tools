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
                         ut::Array<Attachment> in_color_targets,
                         ut::Optional<Attachment> in_depth_stencil_target) : PlatformFramebuffer(ut::Move(platform_framebuffer))
                                                                           , info(framebuffer_info)
                                                                           , color_attachments(ut::Move(in_color_targets))
                                                                           , depth_stencil_attachment(ut::Move(in_depth_stencil_target))
{}

// Move constructor.
Framebuffer::Framebuffer(Framebuffer&&) noexcept = default;

// Move operator.
Framebuffer& Framebuffer::operator =(Framebuffer&&) noexcept = default;

Framebuffer::Attachment::Attachment(Target& in_target,
                                    ut::uint32 in_array_slice,
                                    ut::uint32 in_mip_id) : target(ut::Move(in_target))
                                                          , array_slice(in_array_slice)
                                                          , mip(in_mip_id)
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//