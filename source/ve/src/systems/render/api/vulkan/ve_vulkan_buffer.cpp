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
                               VkRc<vk::memory> memory_rc) : VkRc<vk::buffer>(buffer_handle, device_handle)
                                                           , memory(ut::Move(memory_rc))
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