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
	enum class Rc
	{
		instance,
		device,
		memory,
		dbg_messenger,
		surface,
		swap_chain,
		queue,
		image,
		image_view,
		sampler,
		buffer,
		semaphore,
		cmd_pool,
		cmd_buffer,
		render_pass,
		framebuffer,
		fence,
		shader_module,
		pipeline,
		pipeline_layout,
		descriptor_set_layout,
		descriptor_pool,
		query_pool
	};
}

//----------------------------------------------------------------------------//
// Use specialization ve::render::VkDetail template to define a type of
// the Vulkan resource and a way to delete it.
template<vk::Rc type> class VkDetail;

//----------------------------------------------------------------------------//
// Instance.
template<> struct VkDetail<vk::Rc::instance>
{
	typedef VkInstance Handle;
	static void Destroy(VkInstance handle) { vkDestroyInstance(handle, nullptr); }
};

// Device.
template<> struct VkDetail<vk::Rc::device>
{
	typedef VkDevice Handle;
	static void Destroy(VkDevice handle) { vkDestroyDevice(handle, nullptr); };
};

// Memory.
template<> struct VkDetail<vk::Rc::memory>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE,
	         size_t memory_size = 0) : device(device_handle)
	                                 , size(memory_size)
	{}

	typedef VkDeviceMemory Handle;
	void Destroy(VkDeviceMemory memory_handle) { vkFreeMemory(device, memory_handle, nullptr); }

	size_t GetSize() const
	{
		return size;
	}

private:
	VkDevice device;
	size_t size;
};

// Debug messenger.
template<> struct VkDetail<vk::Rc::dbg_messenger>
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
template<> struct VkDetail<vk::Rc::surface>
{
	VkDetail(VkInstance instance_handle = VK_NULL_HANDLE) : instance(instance_handle)
	{}

	typedef VkSurfaceKHR Handle;
	void Destroy(VkSurfaceKHR surface_handle) { vkDestroySurfaceKHR(instance, surface_handle, nullptr); }

private:
	VkInstance instance;
};

// Swap chain.
template<> struct VkDetail<vk::Rc::swap_chain>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkSwapchainKHR Handle;
	void Destroy(VkSwapchainKHR swapchain_handle) { vkDestroySwapchainKHR(device, swapchain_handle, nullptr); }

private:
	VkDevice device;
};

// Queue.
template<> struct VkDetail<vk::Rc::queue>
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

// Image.
template<> struct VkDetail<vk::Rc::image>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE,
	         VmaAllocator allocator_handle = VK_NULL_HANDLE,
	         VmaAllocation allocation_handle = VK_NULL_HANDLE) : device(device_handle)
	                                                           , allocator(allocator_handle)
	                                                           , allocation(allocation_handle)
	{}

	typedef VkImage Handle;
	void Destroy(VkImage image_handle) { vmaDestroyImage(allocator, image_handle, allocation); }
	VmaAllocation GetAllocation() const { return allocation; };

private:
	VmaAllocator allocator;
	VmaAllocation allocation;
	VkDevice device;
};

// Image view.
template<> struct VkDetail<vk::Rc::image_view>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkImageView Handle;
	void Destroy(VkImageView image_view_handle) { vkDestroyImageView(device, image_view_handle, nullptr); }

private:
	VkDevice device;
};

// Sampler.
template<> struct VkDetail<vk::Rc::sampler>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkSampler Handle;
	void Destroy(VkSampler sampler_handle) { vkDestroySampler(device, sampler_handle, nullptr); }

private:
	VkDevice device;
};

// Buffer.
template<> struct VkDetail<vk::Rc::buffer>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE,
	         VmaAllocator allocator_handle = VK_NULL_HANDLE,
	         VmaAllocation allocation_handle = VK_NULL_HANDLE) : device(device_handle)
	                                                           , allocator(allocator_handle)
	                                                           , allocation(allocation_handle)
	{}

	typedef VkBuffer Handle;
	void Destroy(VkBuffer buffer_handle) { vmaDestroyBuffer(allocator, buffer_handle, allocation); }
	VmaAllocation GetAllocation() const { return allocation; };

private:
	VkDevice device;
	VmaAllocator allocator;
	VmaAllocation allocation;
};

// Semaphore.
template<> struct VkDetail<vk::Rc::semaphore>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkSemaphore Handle;
	void Destroy(VkSemaphore semaphore_handle) { vkDestroySemaphore(device, semaphore_handle, nullptr); }

private:
	VkDevice device;
};

// Command pool.
template<> struct VkDetail<vk::Rc::cmd_pool>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkCommandPool Handle;
	void Destroy(VkCommandPool pool_handle) { vkDestroyCommandPool(device, pool_handle, nullptr); }

private:
	VkDevice device;
};

// Command buffer.
template<> struct VkDetail<vk::Rc::cmd_buffer>
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
template<> struct VkDetail<vk::Rc::render_pass>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkRenderPass Handle;
	void Destroy(VkRenderPass render_pass_handle) { vkDestroyRenderPass(device, render_pass_handle, nullptr); }

private:
	VkDevice device;
};

// Framebuffer.
template<> struct VkDetail<vk::Rc::framebuffer>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkFramebuffer Handle;
	void Destroy(VkFramebuffer framebuffer_handle) { vkDestroyFramebuffer(device, framebuffer_handle, nullptr); }

private:
	VkDevice device;
};

// Fence.
template<> struct VkDetail<vk::Rc::fence>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkFence Handle;
	void Destroy(VkFence fence_handle) { vkDestroyFence(device, fence_handle, nullptr); }

private:
	VkDevice device;
};

// Shader module.
template<> struct VkDetail<vk::Rc::shader_module>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkShaderModule Handle;
	void Destroy(VkShaderModule module_handle) { vkDestroyShaderModule(device, module_handle, nullptr); }

private:
	VkDevice device;
};

// Pipeline.
template<> struct VkDetail<vk::Rc::pipeline>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkPipeline Handle;
	void Destroy(VkPipeline pipeline_handle) { vkDestroyPipeline(device, pipeline_handle, nullptr); }

private:
	VkDevice device;
};

// Pipeline layout.
template<> struct VkDetail<vk::Rc::pipeline_layout>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkPipelineLayout Handle;
	void Destroy(VkPipelineLayout layout_handle) { vkDestroyPipelineLayout(device, layout_handle, nullptr); }

private:
	VkDevice device;
};

// Descriptor set layout.
template<> struct VkDetail<vk::Rc::descriptor_set_layout>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkDescriptorSetLayout Handle;
	void Destroy(VkDescriptorSetLayout layout_handle) { vkDestroyDescriptorSetLayout(device, layout_handle, nullptr); }

private:
	VkDevice device;
};

// Descriptor pool.
template<> struct VkDetail<vk::Rc::descriptor_pool>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkDescriptorPool Handle;
	void Destroy(VkDescriptorPool pool_handle) { vkDestroyDescriptorPool(device, pool_handle, nullptr); }

private:
	VkDevice device;
};

// Query pool.
template<> struct VkDetail<vk::Rc::query_pool>
{
	VkDetail(VkDevice device_handle = VK_NULL_HANDLE) : device(device_handle)
	{}

	typedef VkQueryPool Handle;
	void Destroy(VkQueryPool pool_handle) { vkDestroyQueryPool(device, pool_handle, nullptr); }

private:
	VkDevice device;
};

//----------------------------------------------------------------------------//
// Helper wrapper for Vulkan resources.
template<vk::Rc type>
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