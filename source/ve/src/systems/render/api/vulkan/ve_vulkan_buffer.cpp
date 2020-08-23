//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_buffer.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformBuffer::PlatformBuffer(VkDevice device_handle,
                               VkBuffer buffer_handle,
                               VkDeviceMemory memory_handle) : VkRc<vk::buffer>(buffer_handle, device_handle)
                                                             , memory(memory_handle, device_handle)
{}

// Move constructor.
PlatformBuffer::PlatformBuffer(PlatformBuffer&&) noexcept = default;

// Move operator.
PlatformBuffer& PlatformBuffer::operator =(PlatformBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//