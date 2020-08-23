//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_framebuffer.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformFramebuffer::PlatformFramebuffer(VkDevice device_handle,
                                         VkFramebuffer framebuffer_handle) : VkRc<vk::framebuffer>(framebuffer_handle,
                                                                                                   device_handle)
{}

// Move constructor.
PlatformFramebuffer::PlatformFramebuffer(PlatformFramebuffer&&) noexcept = default;

// Move operator.
PlatformFramebuffer& PlatformFramebuffer::operator =(PlatformFramebuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//