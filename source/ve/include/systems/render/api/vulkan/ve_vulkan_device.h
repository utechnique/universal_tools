//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
#include "systems/render/api/vulkan/ve_vulkan_queue.h"
#include "systems/render/api/vulkan/ve_vulkan_buffer.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Forward declarations.
class Display;

//----------------------------------------------------------------------------//
// Vulkan device.
class PlatformDevice
{
public:
	// Constructor.
	PlatformDevice();

	// Destructor.
	~PlatformDevice();

	// Move constructor.
	PlatformDevice(PlatformDevice&&) noexcept;

	// Move operator.
	PlatformDevice& operator =(PlatformDevice&&) noexcept;

	// Copying is prohibited.
	PlatformDevice(const PlatformDevice&) = delete;
	PlatformDevice& operator =(const PlatformDevice&) = delete;

protected:
	VkRc<vk::instance> instance;
	VkRc<vk::dbg_messenger> dbg_messenger;
	VkRc<vk::device> device;

	// main queue must support all types of commands:
	// graphics, compute, transfer and present
	VkRc<vk::queue> main_queue;

	// command pool for all command buffers created with CmdBufferInfo::usage_once
	// flag and that must be reset every frame by calling ResetCmdPool() method.
	VkRc<vk::cmd_pool> dynamic_cmd_pool;

	// physical device that is used for rendering
	VkPhysicalDevice gpu;

	// queue families available on physical device
	ut::Map<vulkan_queue::FamilyType, vulkan_queue::Family> queue_families;

	// memory properties
	VkPhysicalDeviceMemoryProperties memory_properties;

	// Combines the requirements of the buffer and application requirements
	// to find the right type of memory to use.
	//    @param type_bits - bitmask that contains one bit set for every
	//                       supported memory type for the resource.
	//    @param requirements_mask - requirements flags that must be set for
	//                               the desired memory.
	//    @return - id of the memory or nothing if failed.
	ut::Optional<uint32_t> FindMemoryTypeFromProperties(uint32_t type_bits,
	                                                    VkFlags requirements_mask);

	// Creates vulkan buffer.
	//    @param size - buffer size in bytes.
	//    @param usage - buffer usage flags.
	//    @return - buffer handle or error if failed.
	ut::Result<VkBuffer, ut::Error> CreateVulkanBuffer(VkDeviceSize size,
	                                                   VkBufferUsageFlags usage);

	// Allocates gpu memory for the specified image.
	//    @param buffer - buffer handle.
	//    @param properties - memory properties.
	//    @return - memory resource object or error if failed.
	ut::Result<VkRc<vk::memory>, ut::Error> AllocateImageMemory(VkImage image,
	                                                            VkMemoryPropertyFlags properties);

	// Allocates gpu memory for the specified buffer.
	//    @param buffer - buffer handle.
	//    @param properties - memory properties.
	//    @return - memory resource object or error if failed.
	ut::Result<VkRc<vk::memory>, ut::Error> AllocateBufferMemory(VkBuffer buffer,
	                                                             VkMemoryPropertyFlags properties);

	// Creates staging buffer.
	//    @param size - size of the buffer in bytes.
	//    @param ini_data - optional array of bytes to initialize buffer with.
	//    @return -buffer object or error if failed.
	ut::Result<PlatformBuffer, ut::Error> CreateStagingBuffer(size_t size,
	                                                          ut::Optional<const ut::Array<ut::byte>&> ini_data =
	                                                          ut::Optional<const ut::Array<ut::byte>&>());

	// Copies pixel data to the image subresource according to the specified
	// layout.
	//    @param dst - address of the buffer associated with an image.
	//    @param src - source linear data with zero pitches.
	//    @param pixel_size - size of one pixel in bytes.
	//    @param width - width of an image in pixels.
	//    @param height - height of an image in pixels.
	//    @param depth - depth of an image in pixels.
	//    @param offset - offset in bytes from the @dst.
	//    @param row_pitch - number of bytes between each row in an image.
	//    @param depth_pitch - number of bytes between each slice in an image.
	void CopyPixelsToSubRc(ut::byte* dst,
	                       const ut::byte* src,
	                       size_t pixel_size,
	                       size_t width,
	                       size_t height,
	                       size_t depth,
	                       size_t offset,
	                       size_t row_pitch,
	                       size_t depth_pitch);

	// Copies contents of source buffer to the destination buffer.
	//    @param cmd_buffer - command buffer to record a command.
	//    @param src - source buffer handle.
	//    @param dst - destination buffer handle.
	//    @param size - size of the data to be copied in bytes.
	void CopyVulkanBuffer(VkCommandBuffer cmd_buffer,
	                      VkBuffer src,
	                      VkBuffer dst,
	                      VkDeviceSize size);

	// Copies contents of source buffer to the destination image.
	//    @param cmd_buffer - command buffer to record a command.
	//    @param src - source buffer handle.
	//    @param dst - destination image handle.
	//    @param aspect_mask - bitmask of VkImageAspectFlagBits specifying
	//                         which aspect(s) of the image are included
	//                         in the view.
	//    @param image_layout - current image layout.
	//    @param offset - buffer offset in bytes.
	//    @param width - width of an image in pixels.
	//    @param height - height of an image in pixels.
	//    @param depth - depth of an image in pixels.
	//    @param mip_id - id of the mip to be copied.
	//    @param base_array_layer - id of the first image in an array.
	//    @param layer_count - number of images in an array.
	void CopyVulkanBufferToImage(VkCommandBuffer cmd_buffer,
	                             VkBuffer src,
	                             VkImage dst,
	                             VkImageAspectFlags aspect_mask,
	                             VkImageLayout image_layout,
	                             size_t offset,
	                             ut::uint32 width,
	                             ut::uint32 height,
	                             ut::uint32 depth,
	                             ut::uint32 mip_id,
	                             ut::uint32 base_array_layer,
	                             ut::uint32 layer_count);

	// Creates a command buffer to record immediate commands.
	// This command buffer can be submit by calling EndImmediateCmdBuffer().
	//    @return - command buffer handle or error if failed.
	ut::Result<VkCommandBuffer, ut::Error> BeginImmediateCmdBuffer();

	// Submits provided command buffer and waits for completion.
	//    @param cmd_buffer - command buffer handle.
	//    @return - optional error if failed.
	ut::Optional<ut::Error> EndImmediateCmdBuffer(VkCommandBuffer cmd_buffer);

	// Creates command pool.
	ut::Result<VkCommandPool, ut::Error> CreateCmdPool(VkCommandPoolCreateFlags flags);

private:
	// Creates vkInstance object.
	static VkInstance CreateVulkanInstance();

	// Vulkan message callback.
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDbgCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
	                                                        VkDebugUtilsMessageTypeFlagsEXT,
	                                                        const VkDebugUtilsMessengerCallbackDataEXT*,
	                                                        void*);

	// Returns 'true' if system supports validation layer.
	static bool CheckValidationLayerSupport();

	// Returns physical device that suits best.
	static ut::Optional<VkPhysicalDevice> SelectPreferredPhysicalDevice(const ut::Array<VkPhysicalDevice>& devices);

	// Returns an array of physical devices.
	ut::Array<VkPhysicalDevice> EnumeratePhysicalDevices();

	// Returns a map of queue-type/family-id values.
	ut::Map<vulkan_queue::FamilyType, vulkan_queue::Family> GetQueueFamilies(VkPhysicalDevice gpu);

	// Create intercepter for messages generated by Vulkan instance.
	VkDebugUtilsMessengerEXT CreateDbgMessenger();

	// Creates VkDevice object.
	VkDevice CreateVulkanDevice();

	// Creates desired queue.
	VkRc<vk::queue> CreateQueue(vulkan_queue::FamilyType family_type, uint32_t id);
};
//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
