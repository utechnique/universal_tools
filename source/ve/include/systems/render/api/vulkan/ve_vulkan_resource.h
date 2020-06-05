//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Enumeration of all possible Vulkan resources.
namespace vk
{
	enum Type
	{
		instance,
		device,
		dbg_messenger,
		surface,
		swap_chain,
		queue,
		image_view,
		semaphore,
		cmd_pool,
		cmd_buffer,
		render_pass,
		framebuffer,
		fence
	};
}

//----------------------------------------------------------------------------//
// Use specialization ve::render::VkDetail template to define a type of
// the Vulkan resource and a way to delete it.
template<vk::Type type> class VkDetail;

//----------------------------------------------------------------------------//
// Instance.
template<> struct VkDetail<vk::instance>
{
	typedef VkInstance Handle;
	static void Destroy(VkInstance handle) { vkDestroyInstance(handle, nullptr); }
};

// Device.
template<> struct VkDetail<vk::device>
{
	typedef VkDevice Handle;
	static void Destroy(VkDevice handle) { vkDestroyDevice(handle, nullptr); };
};

// Debug messenger.
template<> struct VkDetail<vk::dbg_messenger>
{
	typedef VkDebugUtilsMessengerEXT Handle;

	VkDetail(VkInstance instance_handle = VK_NULL_HANDLE) : instance(instance_handle)
	{}

	void Destroy(Handle handle)
	{
		auto destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroy_func == nullptr)
		{
			throw ut::Error(ut::error::fail, "vkDestroyDebugUtilsMessengerEXT function is not supported");
		}
		destroy_func(instance, handle, nullptr);
	};

private:
	VkInstance instance;
};

// Surface.
template<> struct VkDetail<vk::surface>
{
	VkDetail(VkInstance instance_handle = VK_NULL_HANDLE) : instance(instance_handle)
	{}

	typedef VkSurfaceKHR Handle;
	void Destroy(VkSurfaceKHR surface_handle) { vkDestroySurfaceKHR(instance, surface_handle, nullptr); }

private:
	VkInstance instance;
};

// Swap chain.
template<> struct VkDetail<vk::swap_chain>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkSwapchainKHR Handle;
	void Destroy(VkSwapchainKHR swapchain_handle) { vkDestroySwapchainKHR(device, swapchain_handle, nullptr); }

private:
	VkDevice device;
};

// Queue.
template<> struct VkDetail<vk::queue>
{
	VkDetail(uint32_t in_queue_id = 0,
	         uint32_t in_queue_family_id = 0) : queue_id(in_queue_id)
	                                          , queue_family_id(in_queue_family_id)
	{}

	typedef VkQueue Handle;
	void Destroy(VkQueue queue_handle) {}
	uint32_t GetQueueFamilyId() { return queue_family_id; }

private:
	uint32_t queue_id;
	uint32_t queue_family_id;
};

// Image view.
template<> struct VkDetail<vk::image_view>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkImageView Handle;
	void Destroy(VkImageView image_view_handle) { vkDestroyImageView(device, image_view_handle, nullptr); }

private:
	VkDevice device;
};

// Image view.
template<> struct VkDetail<vk::semaphore>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkSemaphore Handle;
	void Destroy(VkSemaphore semaphore_handle) { vkDestroySemaphore(device, semaphore_handle, nullptr); }

private:
	VkDevice device;
};

// Command pool.
template<> struct VkDetail<vk::cmd_pool>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkCommandPool Handle;
	void Destroy(VkCommandPool pool_handle) { vkDestroyCommandPool(device, pool_handle, nullptr); }

private:
	VkDevice device;
};

// Command buffer.
template<> struct VkDetail<vk::cmd_buffer>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE,
	         VkCommandPool pool_handle = VK_NULL_HANDLE) : device(device_handle)
	                                                     , cmd_pool(pool_handle)
	{}

	typedef VkCommandBuffer Handle;
	void Destroy(VkCommandBuffer cmd_buffer_handle)
	{
		VkCommandBuffer cmd_buffer_array[1] = { cmd_buffer_handle };
		vkFreeCommandBuffers(device, cmd_pool, 1, cmd_buffer_array);
	}

private:
	VkDevice device;
	VkCommandPool cmd_pool;
};

// Render pass.
template<> struct VkDetail<vk::render_pass>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkRenderPass Handle;
	void Destroy(VkRenderPass render_pass_handle) { vkDestroyRenderPass(device, render_pass_handle, nullptr); }

private:
	VkDevice device;
};

// Framebuffer.
template<> struct VkDetail<vk::framebuffer>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkFramebuffer Handle;
	void Destroy(VkFramebuffer framebuffer_handle) { vkDestroyFramebuffer(device, framebuffer_handle, nullptr); }

private:
	VkDevice device;
};

// Fence.
template<> struct VkDetail<vk::fence>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkFence Handle;
	void Destroy(VkFence fence_handle) { vkDestroyFence(device, fence_handle, nullptr); }

private:
	VkDevice device;
};

//----------------------------------------------------------------------------//
// Helper wrapper for Vulkan resources.
template<vk::Type type>
class VkRc
{
public:
	// Pure Vulkan type of the managed resource.
	typedef typename VkDetail<type>::Handle Handle;

	// Constructor
	VkRc(Handle in_rc = VK_NULL_HANDLE,
	     VkDetail<type> in_detail = VkDetail<type>()) : rc(in_rc)
	                                                  , detail(in_detail)
	{}

	// Destructor
	~VkRc()
	{
		Delete();
	}

	// Move constructor.
	VkRc(VkRc&& other) noexcept : rc(other.rc)
	                            , detail(other.detail)
	{
		other.rc = VK_NULL_HANDLE;
	}

	// Move operator
	VkRc& operator =(VkRc&& other) noexcept
	{
		Delete();
		rc = other.rc;
		detail = ut::Move(other.detail);
		other.rc = VK_NULL_HANDLE;
		return *this;
	}

	// Copying is prohibited.
	VkRc(const VkRc&) = delete;
	VkRc& operator =(const VkRc&) = delete;

	// Destroyes managed object.
	void Delete()
	{
		if (rc)
		{
			detail.Destroy(rc);
			rc = VK_NULL_HANDLE;
		}
	}

	// Sets new resource handle without destroying managed object.
	void Reset(Handle new_rc = VK_NULL_HANDLE,
	           VkDetail<type> new_detail = VkDetail<type>())
	{
		rc = new_rc;
		detail = ut::Move(new_detail);
	}

	// Returns pure Vulkan handle of the managed resource.
	Handle GetVkHandle() const
	{
		return rc;
	}

	// Returns detail object.
	VkDetail<type> GetDetail() const
	{
		return detail;
	}

private:
	Handle rc;
	VkDetail<type> detail;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//