//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan query buffer.
class PlatformQueryBuffer : public VkRc<vk::Rc::query_pool>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformQueryBuffer(VkDevice device_handle,
	                    VkQueryPool guery_pool_handle);

	// Move constructor.
	PlatformQueryBuffer(PlatformQueryBuffer&&) noexcept;

	// Move operator.
	PlatformQueryBuffer& operator =(PlatformQueryBuffer&&) noexcept;

	// Copying is prohibited.
	PlatformQueryBuffer(const PlatformQueryBuffer&) = delete;
	PlatformQueryBuffer& operator =(const PlatformQueryBuffer&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//