//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_cmd_buffer.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformCmdBuffer::PlatformCmdBuffer(VkDevice device_handle,
                                     VkCommandPool cmd_pool_handle,
                                     VkCommandBuffer cmd_buffer_handle,
                                     VkFence fence_handle) : buffer(cmd_buffer_handle,
                                                                    VkDetail<vk::cmd_buffer>(device_handle, cmd_pool_handle))
                                                           , fence(fence_handle, device_handle)
{}

// Move constructor.
PlatformCmdBuffer::PlatformCmdBuffer(PlatformCmdBuffer&&) noexcept = default;

// Move operator.
PlatformCmdBuffer& PlatformCmdBuffer::operator =(PlatformCmdBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//