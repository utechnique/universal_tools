//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_query.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformQueryBuffer::PlatformQueryBuffer(VkDevice device_handle,
                                         VkQueryPool guery_pool_handle) : VkRc<vk::Rc::query_pool>(guery_pool_handle,
                                                                                                   device_handle)
{}

// Move constructor.
PlatformQueryBuffer::PlatformQueryBuffer(PlatformQueryBuffer&&) noexcept = default;

// Move operator.
PlatformQueryBuffer& PlatformQueryBuffer::operator =(PlatformQueryBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//