//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
#include "systems/render/api/vulkan/ve_vulkan_render_target.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan display.
class PlatformDisplay 
{
	friend class Context;
	friend class Device;
public:
	// Constructor.
	PlatformDisplay(VkInstance instance_handle,
	                VkDevice device_handle,
	                VkSurfaceKHR surface_handle,
	                VkSwapchainKHR swap_chain_handle,
	                ut::uint32 buffer_count);

	// Move constructor.
	PlatformDisplay(PlatformDisplay&&) noexcept;

	// Move operator.
	PlatformDisplay& operator =(PlatformDisplay&&) noexcept;

	// Copying is prohibited.
	PlatformDisplay(const PlatformDisplay&) = delete;
	PlatformDisplay& operator =(const PlatformDisplay&) = delete;

protected:
	VkRc<vk::surface> surface;
	VkRc<vk::swap_chain> swap_chain;
	ut::Array< VkRc<vk::semaphore> > availability_semaphores;
	ut::Array< VkRc<vk::semaphore> > present_ready_semaphores;
	VkDevice device;
	ut::uint32 swap_count;

private:
	// Returns an id of the next buffer in a swapchain to be filled.
	ut::uint32 AcquireNextBuffer();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//