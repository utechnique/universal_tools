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
// Vulkan buffer.
class PlatformBuffer : public VkRc<vk::buffer>
{
	friend class Device;
	friend class Context;
public:
	PlatformBuffer(VkDevice device_handle,
	               VkBuffer buffer_handle,
	               VkDeviceMemory memory_handle);

	// Move constructor.
	PlatformBuffer(PlatformBuffer&&) noexcept;

	// Move operator.
	PlatformBuffer& operator =(PlatformBuffer&&) noexcept;

	// Copying is prohibited.
	PlatformBuffer(const PlatformBuffer&) = delete;
	PlatformBuffer& operator =(const PlatformBuffer&) = delete;

private:
	VkRc<vk::memory> memory;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//