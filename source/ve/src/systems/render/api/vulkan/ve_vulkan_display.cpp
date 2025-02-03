//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_display.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformDisplay::PlatformDisplay(VkInstance instance_handle,
                                 VkDevice device_handle,
                                 VkSurfaceKHR surface_handle,
                                 VkSwapchainKHR swap_chain_handle,
                                 ut::uint32 buffer_count) : surface(surface_handle, instance_handle)
                                                          , swap_chain(swap_chain_handle, device_handle)
                                                          , device(device_handle)
                                                          , swap_count(buffer_count-1)
{
	// create semaphores
	for (size_t i = 0; i < buffer_count; i++)
	{
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		VkResult res = vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphore);
		if (res != VK_SUCCESS)
		{
			throw VulkanError(res, "vkCreateSemaphore(display)");
		}
		
		if (!availability_semaphores.Add(VkRc<vk::Rc::semaphore>(semaphore, device)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}

		res = vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphore);
		if (res != VK_SUCCESS)
		{
			throw VulkanError(res, "vkCreateSemaphore(display)");
		}

		if (!present_ready_semaphores.Add(VkRc<vk::Rc::semaphore>(semaphore, device)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
}

// Move constructor.
PlatformDisplay::PlatformDisplay(PlatformDisplay&&) noexcept = default;

// Move operator.
PlatformDisplay& PlatformDisplay::operator =(PlatformDisplay&&) noexcept = default;

// Returns an id of the next buffer in a swapchain to be filled.
ut::uint32 PlatformDisplay::AcquireNextBuffer()
{
	// update swap counter
	swap_count++;
	if (swap_count >= availability_semaphores.Count())
	{
		swap_count = 0;
	}

	// get id of the next buffer
	uint32_t next_buffer_id;
	VkResult res = vkAcquireNextImageKHR(device,
	                                     swap_chain.GetVkHandle(),
	                                     UINT64_MAX,
	                                     availability_semaphores[swap_count].GetVkHandle(),
	                                     VK_NULL_HANDLE,
	                                     &next_buffer_id);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkAcquireNextImageKHR");
	}

	// success
	return next_buffer_id;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//