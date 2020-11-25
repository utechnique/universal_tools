//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_device.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Maximum number of swapchains to present simultaneously.
const ut::uint32 skMaxVkSwapchainPresent = 32;

// Infinite vulkan fence idle.
const uint64_t skFenceInfiniteTimeout = 100000000;

//----------------------------------------------------------------------------//
// Converts compare operation to the one compatible with Vulkan.
VkCompareOp ConvertCompareOpToVulkan(compare::Operation op)
{
	switch (op)
	{
	case compare::never:            return VK_COMPARE_OP_NEVER;
	case compare::less:             return VK_COMPARE_OP_LESS;
	case compare::equal:            return VK_COMPARE_OP_EQUAL;
	case compare::less_or_equal:    return VK_COMPARE_OP_LESS_OR_EQUAL;
	case compare::greater:          return VK_COMPARE_OP_GREATER;
	case compare::not_equal:        return VK_COMPARE_OP_NOT_EQUAL;
	case compare::greater_or_equal: return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case compare::always:           return VK_COMPARE_OP_ALWAYS;
	}
	return VK_COMPARE_OP_ALWAYS;
}

// Converts stencil operation to the one compatible with Vulkan.
VkStencilOp ConvertStencilOpToVulkan(StencilOpState::Operation op)
{
	switch (op)
	{
	case StencilOpState::keep:                return VK_STENCIL_OP_KEEP;
	case StencilOpState::zero:                return VK_STENCIL_OP_ZERO;
	case StencilOpState::replace:             return VK_STENCIL_OP_REPLACE;
	case StencilOpState::increment_and_clamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case StencilOpState::decrement_and_clamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case StencilOpState::invert:              return VK_STENCIL_OP_INVERT;
	case StencilOpState::increment_and_wrap:  return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case StencilOpState::decrement_and_wrap:  return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}
	return VK_STENCIL_OP_KEEP;
}

// Converts blend operation to the one compatible with Vulkan.
VkBlendOp ConvertBlendOpToVulkan(Blending::Operation op)
{
	switch (op)
	{
	case Blending::add:              return VK_BLEND_OP_ADD;
	case Blending::subtract:         return VK_BLEND_OP_SUBTRACT;
	case Blending::reverse_subtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
	case Blending::min:              return VK_BLEND_OP_MIN;
	case Blending::max:              return VK_BLEND_OP_MAX;
	}
	return VK_BLEND_OP_MAX;
}

// Converts blend factor to the one compatible with Vulkan.
VkBlendFactor ConvertBlendFactorToVulkan(Blending::Factor op)
{
	switch (op)
	{
	case Blending::zero:                return VK_BLEND_FACTOR_ZERO;
	case Blending::one:                 return VK_BLEND_FACTOR_ONE;
	case Blending::src_color:           return VK_BLEND_FACTOR_SRC_COLOR;
	case Blending::inverted_src_color:  return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case Blending::dst_color:           return VK_BLEND_FACTOR_DST_COLOR;
	case Blending::inverted_dst_color:  return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case Blending::src_alpha:           return VK_BLEND_FACTOR_SRC_ALPHA;
	case Blending::inverted_src_alpha:  return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case Blending::dst_alpha:           return VK_BLEND_FACTOR_DST_ALPHA;
	case Blending::inverted_dst_alpha:  return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case Blending::src1_color:          return VK_BLEND_FACTOR_SRC1_COLOR;
	case Blending::inverted_src1_color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
	case Blending::src1_alpha:          return VK_BLEND_FACTOR_SRC1_ALPHA;
	case Blending::inverted_src1_alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}
	return VK_BLEND_FACTOR_ONE;
}

// Converts polygon mode to the one compatible with Vulkan.
VkPolygonMode ConvertPolygonModeToVulkan(RasterizationState::PolygonMode mode)
{
	switch (mode)
	{
	case RasterizationState::fill:  return VK_POLYGON_MODE_FILL;
	case RasterizationState::line:  return VK_POLYGON_MODE_LINE;
	case RasterizationState::point: return VK_POLYGON_MODE_POINT;
	}
	return VK_POLYGON_MODE_FILL;
}

// Converts cull mode to the one compatible with Vulkan.
VkCullModeFlagBits ConvertCullModeToVulkan(RasterizationState::CullMode mode)
{
	switch (mode)
	{
	case RasterizationState::no_culling:    return VK_CULL_MODE_NONE;
	case RasterizationState::front_culling: return VK_CULL_MODE_FRONT_BIT;
	case RasterizationState::back_culling:  return VK_CULL_MODE_BACK_BIT;
	}
	return VK_CULL_MODE_NONE;
}

// Converts front face to the one compatible with Vulkan.
VkFrontFace ConvertFrontFaceToVulkan(RasterizationState::FrontFace face)
{
	switch (face)
	{
	case RasterizationState::clockwise: return VK_FRONT_FACE_CLOCKWISE;
	case RasterizationState::counter_clockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
	return VK_FRONT_FACE_CLOCKWISE;
}

// Converts front face to the one compatible with Vulkan.
VkBufferUsageFlagBits ConvertBufferTypeToVulkan(Buffer::Type type)
{
	switch (type)
	{
	case Buffer::vertex: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case Buffer::index: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case Buffer::uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case Buffer::storage: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
}

// Converts image type to the one compatible with Vulkan.
VkImageType ConvertImageTypeToVulkan(Image::Type type)
{
	switch (type)
	{
	case Image::type_1D: return VK_IMAGE_TYPE_1D;
	case Image::type_2D: return VK_IMAGE_TYPE_2D;
	case Image::type_cube: return VK_IMAGE_TYPE_2D;
	case Image::type_3D: return VK_IMAGE_TYPE_3D;
	}
	return VK_IMAGE_TYPE_2D;
}

// Converts image type to the one compatible with Vulkan.
VkImageViewType ConvertImageViewTypeToVulkan(Image::Type type)
{
	switch (type)
	{
	case Image::type_1D: return VK_IMAGE_VIEW_TYPE_1D;
	case Image::type_2D: return VK_IMAGE_VIEW_TYPE_2D;
	case Image::type_cube: return VK_IMAGE_VIEW_TYPE_CUBE;
	case Image::type_3D: return VK_IMAGE_VIEW_TYPE_3D;
	}
	return VK_IMAGE_VIEW_TYPE_2D;
}

// Converts texture filter to the one compatible with Vulkan.
VkFilter ConvertFilterToVulkan(Sampler::Filter filter)
{
	switch (filter)
	{
	case Sampler::filter_nearest: return VK_FILTER_NEAREST;
	case Sampler::filter_linear: return VK_FILTER_LINEAR;
	}
	return  VK_FILTER_NEAREST;
}

// Converts mipmap mode to the one compatible with Vulkan.
VkSamplerMipmapMode ConvertMipmapModeToVulkan(Sampler::Filter filter)
{
	switch (filter)
	{
	case Sampler::filter_nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	case Sampler::filter_linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
	return  VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

// Converts sampler address mode to the one compatible with Vulkan.
VkSamplerAddressMode ConvertAddressModeToVulkan(Sampler::AddressMode mode)
{
	switch (mode)
	{
	case Sampler::address_wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case Sampler::address_mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case Sampler::address_clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case Sampler::address_border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	}
	return  VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

//----------------------------------------------------------------------------//
// Constructor.
PlatformDevice::PlatformDevice() : instance(CreateVulkanInstance())
                                 , gpu(nullptr)
{
	// create dbg messenger
	if (skEnableVulkanValidationLayer)
	{
		dbg_messenger = VkRc<vk::dbg_messenger>(CreateDbgMessenger(), instance.GetVkHandle());
	}

	// create device
	device = VkRc<vk::device>(CreateVulkanDevice());

	// create queues
	main_queue = CreateQueue(vulkan_queue::main, 0);

	// command pools
	ut::Result<VkCommandPool, ut::Error> pool_result = CreateCmdPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	dynamic_cmd_pool = VkRc<vk::cmd_pool>(pool_result.Get(), device.GetVkHandle());

	// memory properties
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
}

// Destructor.
PlatformDevice::~PlatformDevice()
{
	vkDeviceWaitIdle(device.GetVkHandle());
}

// Move constructor.
PlatformDevice::PlatformDevice(PlatformDevice&&) noexcept = default;

// Move operator.
PlatformDevice& PlatformDevice::operator =(PlatformDevice&&) noexcept = default;

// Combines the requirements of the buffer and application requirements
// to find the right type of memory to use.
//    @param type_bits - bitmask that contains one bit set for every
//                       supported memory type for the resource.
//    @param requirements_mask - requirements flags that must be set for
//                               the desired memory.
//    @return - id of the memory or nothing if failed.
ut::Optional<uint32_t> PlatformDevice::FindMemoryTypeFromProperties(uint32_t type_bits,
                                                                    VkFlags requirements_mask)
{
	// search memtypes to find first index with those properties
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
	{
		if ((type_bits & 1) == 1)
		{
			// type is available, does it match user properties?
			if ((memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
			{
				return i;
			}
		}
		type_bits >>= 1;
	}

	// no memory types matched, return failure
	return ut::Optional<uint32_t>();
}

// Creates vulkan buffer.
//    @param size - buffer size in bytes.
//    @param usage - buffer usage flags.
//    @return - buffer handle or error if failed.
ut::Result<VkBuffer, ut::Error> PlatformDevice::CreateVulkanBuffer(VkDeviceSize size,
                                                                   VkBufferUsageFlags usage)
{
	// initialize VkBufferCreateInfo
	VkBufferCreateInfo buf_info;
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.pNext = nullptr;
	buf_info.usage = usage;
	buf_info.size = size;
	buf_info.queueFamilyIndexCount = 0;
	buf_info.pQueueFamilyIndices = nullptr;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buf_info.flags = 0;

	// create buffer
	VkBuffer buffer;
	VkResult res = vkCreateBuffer(device.GetVkHandle(), &buf_info, nullptr, &buffer);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateBuffer"));
	}

	// success
	return buffer;
}

// Allocates gpu memory for the specified image.
//    @param buffer - buffer handle.
//    @param properties - memory properties.
//    @return - memory resource object or error if failed.
ut::Result<VkRc<vk::memory>, ut::Error> PlatformDevice::AllocateImageMemory(VkImage image,
                                                                            VkMemoryPropertyFlags properties)
{
	// get memory requirements
	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(device.GetVkHandle(), image, &mem_reqs);

	// get memory type
	ut::Optional<uint32_t> memory_type = FindMemoryTypeFromProperties(mem_reqs.memoryTypeBits, properties);
	if (!memory_type)
	{
		return ut::MakeError(ut::Error(ut::error::not_found, "Vulkan: no suitable memory type for the provided buffer."));
	}

	// initialize VkMemoryAllocateInfo
	VkMemoryAllocateInfo alloc_info;
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = memory_type.Get();

	// allocate video memory
	VkDeviceMemory image_memory;
	VkResult res = vkAllocateMemory(device.GetVkHandle(), &alloc_info, nullptr, &image_memory);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkAllocateMemory(buffer)"));
	}

	// bind memory
	vkBindImageMemory(device.GetVkHandle(), image, image_memory, 0);

	// success
	VkDetail<vk::memory> detail(device.GetVkHandle(), alloc_info.allocationSize);
	return VkRc<vk::memory>(image_memory, detail);
}

// Allocates gpu memory for the specified buffer.
//    @param buffer - buffer handle.
//    @param properties - memory properties.
//    @return - memory resource object or error if failed.
ut::Result<VkRc<vk::memory>, ut::Error> PlatformDevice::AllocateBufferMemory(VkBuffer buffer,
                                                                             VkMemoryPropertyFlags properties)
{
	// get memory requirements
	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(device.GetVkHandle(), buffer, &mem_reqs);

	// get memory type
	ut::Optional<uint32_t> memory_type = FindMemoryTypeFromProperties(mem_reqs.memoryTypeBits, properties);
	if (!memory_type)
	{
		return ut::MakeError(ut::Error(ut::error::not_found, "Vulkan: no suitable memory type for the provided buffer."));
	}

	// initialize VkMemoryAllocateInfo
	VkMemoryAllocateInfo alloc_info;
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.allocationSize = mem_reqs.size;
	alloc_info.memoryTypeIndex = memory_type.Get();

	// allocate video memory
	VkDeviceMemory buffer_memory;
	VkResult res = vkAllocateMemory(device.GetVkHandle(), &alloc_info, nullptr, &buffer_memory);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkAllocateMemory(buffer)"));
	}

	// bind memory
	vkBindBufferMemory(device.GetVkHandle(), buffer, buffer_memory, 0);

	// success
	VkDetail<vk::memory> detail(device.GetVkHandle(), alloc_info.allocationSize);
	return VkRc<vk::memory>(buffer_memory, detail);
}

// Creates staging buffer.
//    @param size - size of the buffer in bytes.
//    @param ini_data - optional array of bytes to initialize buffer with.
//    @return -buffer object or error if failed.
ut::Result<PlatformBuffer, ut::Error> PlatformDevice::CreateStagingBuffer(size_t size,
                                                                          ut::Optional<const ut::Array<ut::byte>&> ini_data)
{
	// create staging buffer
	ut::Result<VkBuffer, ut::Error> staging_buffer_result = CreateVulkanBuffer(size,
	                                                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	if (!staging_buffer_result)
	{
		return ut::MakeError(staging_buffer_result.MoveAlt());
	}
	VkBuffer staging_buffer = staging_buffer_result.Get();

	// allocate staging memory
	VkMemoryPropertyFlags staging_mem_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	ut::Result<VkRc<vk::memory>, ut::Error> staging_memory = AllocateBufferMemory(staging_buffer,
	                                                                              staging_mem_properties);
	if (!staging_memory)
	{
		return ut::MakeError(staging_memory.MoveAlt());
	}

	// copy data to the staging buffer
	if(ini_data)
	{
		const size_t ini_size = ut::Min<size_t>(ini_data->GetSize(), size);
		void* staging_buffer_data;
		VkResult res = vkMapMemory(device.GetVkHandle(),
								   staging_memory->GetVkHandle(),
								   0, ini_size,
								   0, &staging_buffer_data);
		if (res != VK_SUCCESS)
		{
			return ut::MakeError(VulkanError(res, "vkMapMemory(staging buffer)"));
		}
		ut::memory::Copy(staging_buffer_data, ini_data->GetAddress(), ini_size);
		vkUnmapMemory(device.GetVkHandle(), staging_memory->GetVkHandle());
	}

	// success
	return PlatformBuffer(device.GetVkHandle(),
	                      staging_buffer,
	                      staging_memory.Move());
}

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
void PlatformDevice::CopyPixelsToSubRc(ut::byte* dst,
                                       const ut::byte* src,
                                       size_t pixel_size,
                                       size_t width,
                                       size_t height,
                                       size_t depth,
                                       size_t offset,
                                       size_t row_pitch,
                                       size_t depth_pitch)
{
	const size_t row_size = width * pixel_size;
	for (size_t z = 0; z < depth; z++)
	{
		ut::byte* row = dst + offset + depth_pitch * z;
		for (size_t y = 0; y < height; y++)
		{
			ut::memory::Copy(row, src, row_size);
			row += row_pitch;
			src += row_size;
		}
	}
}

// Copies contents of source buffer to the destination buffer.
//    @param cmd_buffer - command buffer to record a command.
//    @param src - source buffer handle.
//    @param dst - destination buffer handle.
//    @param size - size of the data to be copied in bytes.
void PlatformDevice::CopyVulkanBuffer(VkCommandBuffer cmd_buffer,
                                      VkBuffer src,
                                      VkBuffer dst,
                                      VkDeviceSize size)
{
	UT_ASSERT(src != VK_NULL_HANDLE);
	UT_ASSERT(dst != VK_NULL_HANDLE);

	VkBufferCopy copy_region;
	copy_region.srcOffset = 0; // Optional
	copy_region.dstOffset = 0; // Optional
	copy_region.size = size;
	vkCmdCopyBuffer(cmd_buffer, src, dst, 1, &copy_region);
}

// Copies contents of source buffer to the destination image.
//    @param cmd_buffer - command buffer to record a command.
//    @param src - source buffer handle.
//    @param dst - destination image handle.
//    @param offset - buffer offset in bytes.
//    @param aspect_mask - bitmask of VkImageAspectFlagBits specifying
//                         which aspect(s) of the image are included
//                         in the view.
//    @param image_layout - current image layout.
//    @param width - width of an image in pixels.
//    @param height - height of an image in pixels.
//    @param depth - depth of an image in pixels.
//    @param mip_id - id of the mip to be copied.
//    @param base_array_layer - id of the first image in an array.
//    @param layer_count - number of images in an array.
void PlatformDevice::CopyVulkanBufferToImage(VkCommandBuffer cmd_buffer,
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
                                             ut::uint32 layer_count)
{
	VkBufferImageCopy copy_region;
	copy_region.bufferOffset = offset;
	copy_region.bufferRowLength = width;
	copy_region.bufferImageHeight = height;
	copy_region.imageSubresource.aspectMask = aspect_mask;
	copy_region.imageSubresource.mipLevel = mip_id;
	copy_region.imageSubresource.baseArrayLayer = base_array_layer;
	copy_region.imageSubresource.layerCount = layer_count;
	copy_region.imageOffset.x = 0;
	copy_region.imageOffset.y = 0;
	copy_region.imageOffset.z = 0;
	copy_region.imageExtent.width = width;
	copy_region.imageExtent.height = height;
	copy_region.imageExtent.depth = depth;

	vkCmdCopyBufferToImage(cmd_buffer, src, dst, image_layout, 1, &copy_region);
}

// Creates a command buffer to record immediate commands.
// This command buffer can be submit by calling SubmitImmediateCmdBuffer().
//    @return - command buffer handle or error if failed.
ut::Result<VkCommandBuffer, ut::Error> PlatformDevice::BeginImmediateCmdBuffer()
{
	VkCommandBufferAllocateInfo alloc_info;
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = dynamic_cmd_pool.GetVkHandle();
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer cmd_buffer;
	VkResult res = vkAllocateCommandBuffers(device.GetVkHandle(),
	                                        &alloc_info,
	                                        &cmd_buffer);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkAllocateCommandBuffers(immediate)"));
	}

	VkCommandBufferBeginInfo begin_info;
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pNext = nullptr;
	begin_info.pInheritanceInfo = nullptr;

	res = vkBeginCommandBuffer(cmd_buffer, &begin_info);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkBeginCommandBuffer(immediate)"));
	}

	return cmd_buffer;
}

// Submits provided command buffer and waits for completion.
//    @param cmd_buffer - command buffer handle.
//    @return - optional error if failed.
ut::Optional<ut::Error> PlatformDevice::EndImmediateCmdBuffer(VkCommandBuffer cmd_buffer)
{
	VkResult res = vkEndCommandBuffer(cmd_buffer);
	if (res != VK_SUCCESS)
	{
		return VulkanError(res, "vkEndCommandBuffer(immediate)");
	}

	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = nullptr;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buffer;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = nullptr;
	submit_info.pWaitDstStageMask = nullptr;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = nullptr;

	VkFenceCreateInfo fence_info;
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = 0;

	VkFence fence;
	res = vkCreateFence(device.GetVkHandle(), &fence_info, nullptr, &fence);
	if (res != VK_SUCCESS)
	{
		return VulkanError(res, "vkCreateFence(immediate cmd buffer)");
	}

	res = vkQueueSubmit(main_queue.GetVkHandle(), 1, &submit_info, fence);
	if (res != VK_SUCCESS)
	{
		return VulkanError(res, "vkQueueSubmit(immediate cmd buffer)");
	}

	do
	{
		res = vkWaitForFences(device.GetVkHandle(), 1, &fence, VK_TRUE, skFenceInfiniteTimeout);
	} while (res == VK_TIMEOUT);
	UT_ASSERT(res == VK_SUCCESS);
	vkDestroyFence(device.GetVkHandle(), fence, nullptr);

	vkFreeCommandBuffers(device.GetVkHandle(),
	                     dynamic_cmd_pool.GetVkHandle(),
	                     1,
	                     &cmd_buffer);

	return ut::Optional<ut::Error>();
}

// Creates vkInstance object.
VkInstance PlatformDevice::CreateVulkanInstance()
{
	// name of the application/engine
	const char* app_name = "virtual_environment";

	// initialize the VkApplicationInfo structure
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = app_name;
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = app_name;
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_1;

	// instance information object
	VkInstanceCreateInfo inst_info = {};

	// get vulkan extensions specific to the current platform
	ut::Array<const char*> extensions = GetPlatformVkInstanceExtensions();

	// possible layer wrappers
	ut::Array<const char*> layers;

	// initialize validation layer
	if (skEnableVulkanValidationLayer)
	{
		if (CheckValidationLayerSupport())
		{
			layers.Add(skVulkanValidationLayerName);
			extensions.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			ut::log.Lock() << "Vulkan: validation layer enabled." << ut::cret;
		}
		else
		{
			ut::log.Lock() << "Vulkan: validation layer is not supported." << ut::cret;
		}
	}

	// initialize the VkInstanceCreateInfo structure
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = static_cast<uint32_t>(extensions.GetNum());
	inst_info.ppEnabledExtensionNames = extensions.GetAddress();
	inst_info.enabledLayerCount = static_cast<uint32_t>(layers.GetNum());
	inst_info.ppEnabledLayerNames = layers.GetAddress();

	// create instance itself
	VkInstance out_instance;
	VkResult res = vkCreateInstance(&inst_info, nullptr, &out_instance);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		throw ut::Error(ut::error::not_supported, "Cannot find a compatible Vulkan ICD.");
	}
	else if (res == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan extension is not supported.");
	}
	else if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateInstance");
	}

	// success
	return out_instance;
}

// Returns 'true' if system supports validation layer.
bool PlatformDevice::CheckValidationLayerSupport()
{
	// get number of layers
	uint32_t layer_count;
	VkResult res = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumerateInstanceLayerProperties(count)");
	}

	// get layer properties
	ut::Array<VkLayerProperties> available_layers(layer_count);
	res = vkEnumerateInstanceLayerProperties(&layer_count, available_layers.GetAddress());
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumerateInstanceLayerProperties(properties)");
	}

	// check if our validation layer name matches any
	for (size_t i = 0; i < layer_count; i++)
	{
		if (ut::StrCmp<char>(skVulkanValidationLayerName, available_layers[i].layerName))
		{
			return true;
		}
	}

	// not found
	return false;
}

// Returns physical device that suits best.
ut::Optional<VkPhysicalDevice> PlatformDevice::SelectPreferredPhysicalDevice(const ut::Array<VkPhysicalDevice>& devices)
{
	if (devices.GetNum() == 0)
	{
		throw ut::Error(ut::error::empty, "Gpu list is empty");
	}

	ut::Optional< ut::Pair<VkPhysicalDevice, ut::uint32> > top_scored_device;

	for (size_t i = 0; i < devices.GetNum(); i++)
	{
		ut::uint32 score = 0;

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(devices[i], &properties);
		vkGetPhysicalDeviceFeatures(devices[i], &features);

		// check features
		if (!features.geometryShader ||
			!features.tessellationShader ||
			!features.fillModeNonSolid)
		{
			continue;
		}

		// it's better to use discrete gpu
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1;
		}

		if (!top_scored_device || score > top_scored_device->second)
		{
			top_scored_device = ut::Pair<VkPhysicalDevice, ut::uint32>(devices[i], score);
		}		
	}

	if (top_scored_device)
	{
		return top_scored_device->first;
	}

	return ut::Optional<VkPhysicalDevice>();
}

// Vulkan message callback.
VKAPI_ATTR VkBool32 VKAPI_CALL PlatformDevice::VulkanDbgCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT type,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                 void* user_data)
{
	ut::log.Lock() << "Vulkan: " << callback_data->pMessage << ut::cret;
	if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		throw ut::Error(ut::error::fail, callback_data->pMessage);
	}
	return VK_FALSE;
}

// Returns an array of physical devices.
ut::Array<VkPhysicalDevice> PlatformDevice::EnumeratePhysicalDevices()
{
	// get number of physical devices
	uint32_t gpu_count = 1;
	VkResult res = vkEnumeratePhysicalDevices(instance.GetVkHandle(), &gpu_count, nullptr);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumeratePhysicalDevices(count)");
	}
	else if (gpu_count == 0)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan can't detect any gpu adapter.");
	}

	// enumerate physical devices
	ut::Array<VkPhysicalDevice> physical_devices(gpu_count);
	res = vkEnumeratePhysicalDevices(instance.GetVkHandle(), &gpu_count, physical_devices.GetAddress());
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumeratePhysicalDevices");
	}
	else if (gpu_count == 0)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan can't enumerate physical devices.");
	}

	return physical_devices;
}

// Returns an array of queue family properties.
ut::Map<vulkan_queue::FamilyType, vulkan_queue::Family> PlatformDevice::GetQueueFamilies(VkPhysicalDevice physical_device)
{
	// get number of queue families
	uint32_t queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	if (queue_family_count == 0)
	{
		throw ut::Error(ut::error::fail, "vkGetPhysicalDeviceQueueFamilyProperties returned zero.");
	}

	// get all families
	ut::Array<VkQueueFamilyProperties> properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, properties.GetAddress());
	if (queue_family_count == 0)
	{
		throw ut::Error(ut::error::fail, "vkGetPhysicalDeviceQueueFamilyProperties returned empty list of properties.");
	}

	// select needed
	ut::Map<vulkan_queue::FamilyType, vulkan_queue::Family> out_map;
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if (properties[i].queueCount == 0)
		{
			continue;
		}

		vulkan_queue::Family family;
		family.id = i;
		family.count = properties[i].queueCount;

		// main
		if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
		    properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
		    properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			out_map.Insert(vulkan_queue::main, ut::Move(family));
		} // compute
		else if (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
		         !(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		         !(properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			out_map.Insert(vulkan_queue::compute, ut::Move(family));
		} // transfer
		else if (properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
		         !(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		         !(properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			out_map.Insert(vulkan_queue::compute, ut::Move(family));
		}
	}

	// success
	return out_map;
}

// Create intercepter for messages generated by Vulkan instance.
VkDebugUtilsMessengerEXT PlatformDevice::CreateDbgMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT messenger_info{};
	messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messenger_info.pfnUserCallback = PlatformDevice::VulkanDbgCallback;
	messenger_info.pUserData = nullptr; // Optional
	
	auto create_messenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance.GetVkHandle(), "vkCreateDebugUtilsMessengerEXT");
	if (create_messenger == nullptr)
	{
		throw ut::Error(ut::error::not_supported, "vkCreateDebugUtilsMessengerEXT function is not supported");
	}

	VkDebugUtilsMessengerEXT out_messenger;
	VkResult res = create_messenger(instance.GetVkHandle(), &messenger_info, nullptr, &out_messenger);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateDebugUtilsMessengerEXT");
	}
	
	return out_messenger;
}

// Creates VkDevice object.
VkDevice PlatformDevice::CreateVulkanDevice()
{
	// enumerate physical devices
	ut::Array<VkPhysicalDevice> physical_devices = EnumeratePhysicalDevices();

	// select suitable gpu
	ut::Optional<VkPhysicalDevice> preferred_gpu = SelectPreferredPhysicalDevice(physical_devices);
	if (!preferred_gpu)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan: no suitable GPU.");
	}

	// extract gpu name
	gpu = preferred_gpu.Get();
	VkPhysicalDeviceProperties gpu_properties;
	vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
	ut::log.Lock() << "Vulkan: using " << gpu_properties.deviceName << " for rendering." << ut::cret;

	// retrieve queue family properties
	queue_families = PlatformDevice::GetQueueFamilies(gpu);

	// find graphics queue id
	ut::Optional<vulkan_queue::Family&> main_queue_family  = queue_families.Find(vulkan_queue::main);

	// validate graphics queue family
	if (!main_queue_family)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan: General purpose queue is not supported.");
	}

	// VkDeviceQueueCreateInfo
	float queue_priorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queue_info = {};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = nullptr;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;
	queue_info.queueFamilyIndex = main_queue_family->id;

	// get device extensions
	ut::Array<const char*> extensions = GetDeviceVkInstanceExtensions();

	// enumerate features to be enabled
	VkPhysicalDeviceFeatures features;
	ut::memory::Set(&features, 0, sizeof(VkPhysicalDeviceFeatures));
	features.geometryShader = VK_TRUE;
	features.tessellationShader = VK_TRUE;
	features.fillModeNonSolid = VK_TRUE;

	// VkDeviceCreateInfo
	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = nullptr;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = static_cast<uint32_t>(extensions.GetNum());
	device_info.ppEnabledExtensionNames = extensions.GetAddress();
	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = nullptr;
	device_info.pEnabledFeatures = &features;

	// create device
	VkDevice out_device;
	VkResult res = vkCreateDevice(gpu, &device_info, nullptr, &out_device);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateDevice");
	}

	// success
	return out_device;
}

// Creates desired queue.
VkRc<vk::queue> PlatformDevice::CreateQueue(vulkan_queue::FamilyType family_type, uint32_t queue_id)
{
	ut::Optional<vulkan_queue::Family&> family = queue_families.Find(family_type);
	if (!family)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan: queue type is not supported.");
	}

	const uint32_t family_id = family->id;

	VkQueue queue;
	vkGetDeviceQueue(device.GetVkHandle(), family_id, queue_id, &queue);

	VkDetail<vk::queue> detail(queue_id, family_id);
	return VkRc<vk::queue>(queue, detail);
}

// Creates command pool.
ut::Result<VkCommandPool, ut::Error> PlatformDevice::CreateCmdPool(VkCommandPoolCreateFlags flags)
{
	ut::Optional<vulkan_queue::Family&> main_queue_family = queue_families.Find(vulkan_queue::main);
	if (!main_queue_family)
	{
		return ut::MakeError(ut::Error(ut::error::not_supported, "Vulkan: General purpose queue is not supported."));
	}

	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = nullptr;
	cmd_pool_info.queueFamilyIndex = main_queue_family->id;
	cmd_pool_info.flags = flags;

	VkCommandPool out_cmd_pool;
	VkResult res = vkCreateCommandPool(device.GetVkHandle(), &cmd_pool_info, nullptr, &out_cmd_pool);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateCommandPool"));
	}

	return out_cmd_pool;
}

//----------------------------------------------------------------------------//
// Converts load operation value to vulkan format.
VkAttachmentLoadOp ConvertLoadOpToVulkan(RenderTargetSlot::LoadOperation load_op)
{
	switch (load_op)
	{
		case RenderTargetSlot::load_extract: return VK_ATTACHMENT_LOAD_OP_LOAD;
		case RenderTargetSlot::load_clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case RenderTargetSlot::load_dont_care: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

// Converts store operation value to vulkan format.
VkAttachmentStoreOp ConvertStoreOpToVulkan(RenderTargetSlot::StoreOperation store_op)
{
	switch (store_op)
	{
		case RenderTargetSlot::store_save: return VK_ATTACHMENT_STORE_OP_STORE;
		case RenderTargetSlot::store_dont_care: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

//----------------------------------------------------------------------------//
// Constructor.
Device::Device(ut::SharedPtr<ui::Frontend::Thread> ui_frontend) : PlatformDevice()
{}

// Move constructor.
Device::Device(Device&&) noexcept = default;

// Move operator.
Device& Device::operator =(Device&&) noexcept = default;

// Creates a new image.
//    @param info - reference to the Image::Info object describing an image.
//    @return - new image object of error if failed.
ut::Result<Image, ut::Error> Device::CreateImage(Image::Info info)
{
	// dimensions
	bool is_1d = info.type == Image::type_1D;
	bool is_2d = info.type == Image::type_2D || info.type == Image::type_cube;
	bool is_3d = info.type == Image::type_3D;
	bool is_cube = info.type == Image::type_cube;

	// pixel format
	const VkFormat pixel_format = ConvertPixelFormatToVulkan(info.format);

	// figure out pixel size
	const ut::uint32 pixel_size = pixel::GetSize(info.format);
	if (pixel_size == 0)
	{
		return ut::MakeError(ut::error::invalid_arg, "Vulkan: Cannot create image with zero pixel size.");
	}

	// check if staging is required
	const size_t ini_size = info.data.GetSize();
	const bool must_be_initialized = ini_size != 0;
	const bool must_have_cpu_access = info.usage == render::memory::gpu_read_cpu_write ||
	                                  info.usage == render::memory::gpu_read_write_cpu_staging;
	const bool tiling_must_be_linear = must_have_cpu_access;
	const bool needs_staging = info.usage != render::memory::gpu_read_cpu_write;

	// check if image is used as a render target
	const bool is_render_target = info.usage == render::memory::gpu_read_write;

	// check if gpu supports linear tiling
	if (tiling_must_be_linear)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(gpu, pixel_format, &formatProps);
		if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
		{
			ut::String err_str("Vulkan: image cannot be create - GPU doesn't support linear tiling.");
			return ut::MakeError(ut::error::not_supported, ut::Move(err_str));
		}
	}

	// check if image is a depth buffer
	const bool is_depth_buffer = pixel::IsDepthFormat(info.format);

	// check if image is a stencil buffer
	const bool is_stencil_buffer = pixel::IsStencilFormat(info.format);

	// image state
	PlatformImage::State image_state = PlatformImage::State::CreateForShaderResource();
	if (is_render_target)
	{
		image_state = is_depth_buffer ?
		              PlatformImage::State::CreateForDepthStencilTarget() :
		              PlatformImage::State::CreateForColorTarget();
	}

	// aspect mask
	VkImageAspectFlags aspect_mask = is_depth_buffer ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	if (is_stencil_buffer)
	{
		aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	// determine image usage
	VkImageUsageFlags image_usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	if (is_render_target)
	{
		image_usage |= (is_depth_buffer || is_stencil_buffer) ?
		               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT :
		               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	// initialize info
	VkImageCreateInfo image_info;
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = nullptr;
	image_info.flags = is_cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
	image_info.imageType = ConvertImageTypeToVulkan(info.type);
	image_info.format = pixel_format;
	image_info.extent.width = info.width;
	image_info.extent.height = info.height;
	image_info.extent.depth = info.depth;
	image_info.mipLevels = info.mip_count;
	image_info.arrayLayers = is_cube ? 6 : 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = tiling_must_be_linear ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = image_usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = nullptr;
	image_info.initialLayout = (!needs_staging && must_be_initialized) ? VK_IMAGE_LAYOUT_PREINITIALIZED :
	                                                                     VK_IMAGE_LAYOUT_UNDEFINED;

	// set valid usage for the buffer
	if (needs_staging)
	{
		image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	// track image access flags
	VkAccessFlags current_img_access = 0;

	// create image
	VkImage image;
	VkResult res = vkCreateImage(device.GetVkHandle(), &image_info, nullptr, &image);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateImage"));
	}

	// allocate memory
	VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if (needs_staging)
	{
		memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	ut::Result<VkRc<vk::memory>, ut::Error> memory_result = AllocateImageMemory(image, memory_properties);
	if (!memory_result)
	{
		return ut::MakeError(memory_result.MoveAlt());
	}
	VkRc<vk::memory> image_memory(memory_result.Move());
	const size_t image_size = image_memory.GetDetail().GetSize();

	// start immediate commands
	ut::Result<VkCommandBuffer, ut::Error> immediate_buffer = BeginImmediateCmdBuffer();
	if (!immediate_buffer)
	{
		return ut::MakeError(immediate_buffer.MoveAlt());
	}

	// image may need staging buffer
	ut::Optional<PlatformBuffer> staging_buffer;
	if (needs_staging)
	{
		// create staging buffer
		ut::Result<PlatformBuffer, ut::Error> staging_buffer_result = CreateStagingBuffer(image_size);
		if (!staging_buffer_result)
		{
			return ut::MakeError(staging_buffer_result.MoveAlt());
		}
		staging_buffer = staging_buffer_result.Move();

		// set image layout for staging
		VkImageMemoryBarrier staging_barrier;
		staging_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		staging_barrier.pNext = nullptr;
		staging_barrier.srcAccessMask = current_img_access;
		staging_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		staging_barrier.oldLayout = image_info.initialLayout;
		staging_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		staging_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		staging_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		staging_barrier.image = image;
		staging_barrier.subresourceRange.aspectMask = aspect_mask;
		staging_barrier.subresourceRange.baseMipLevel = 0;
		staging_barrier.subresourceRange.levelCount = image_info.mipLevels;
		staging_barrier.subresourceRange.baseArrayLayer = 0;
		staging_barrier.subresourceRange.layerCount = image_info.arrayLayers;
		vkCmdPipelineBarrier(immediate_buffer.Get(),
		                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		                     VK_PIPELINE_STAGE_TRANSFER_BIT,
		                     0, 0, nullptr, 0, nullptr,
		                     1, &staging_barrier);

		// update layout information
		current_img_access = staging_barrier.dstAccessMask;
		image_info.initialLayout = staging_barrier.newLayout;
	}

	// copy memory
	if (must_be_initialized)
	{
		// get address of the buffer to initialize image, it can be
		// staging buffer or direct image memory
		VkDeviceMemory mapped_memory;
		if (needs_staging)
		{
			mapped_memory = staging_buffer->memory.GetVkHandle();
		}
		else
		{
			mapped_memory = image_memory.GetVkHandle();
		}

		// map image memory
		void* image_data;
		VkResult res = vkMapMemory(device.GetVkHandle(), mapped_memory, 0, image_size, 0, &image_data);
		if (res != VK_SUCCESS)
		{
			return ut::MakeError(VulkanError(res, "vkMapMemory(initializing image)"));
		}

		// initialize all subresources
		ut::byte* mip_data = info.data.GetAddress();
		const ut::uint32 cubeface_count = is_cube ? 6 : 1;
		for (ut::uint32 cubeface = 0; cubeface < cubeface_count; cubeface++)
		{
			ut::uint32 mip_width = info.width;
			ut::uint32 mip_height = info.height;
			ut::uint32 mip_depth = info.depth;

			// iterate all mips
			for (ut::uint32 mip = 0; mip < info.mip_count; mip++)
			{
				// get pitches (row, depth, array) for the current mip
				VkSubresourceLayout layout;
				VkImageSubresource subrc;
				subrc.aspectMask = aspect_mask;
				subrc.mipLevel = mip;
				subrc.arrayLayer = cubeface;
				if (tiling_must_be_linear)
				{
					vkGetImageSubresourceLayout(device.GetVkHandle(), image, &subrc, &layout);
				}
				else
				{
					layout.offset = mip_data - info.data.GetAddress();
					layout.rowPitch = mip_width * pixel_size;
					layout.depthPitch = layout.rowPitch * mip_height;
				}

				// copy data
				CopyPixelsToSubRc(static_cast<ut::byte*>(image_data),
				                  mip_data,
				                  pixel_size,
				                  mip_width,
				                  mip_height,
				                  mip_depth,
				                  layout.offset,
				                  layout.rowPitch,
				                  layout.depthPitch);

				// transfer data to the gpu
				if (needs_staging)
				{
					CopyVulkanBufferToImage(immediate_buffer.Get(),
					                        staging_buffer->GetVkHandle(),
					                        image,
					                        aspect_mask,
					                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					                        layout.offset,
					                        mip_width,
					                        mip_height,
					                        mip_depth,
					                        mip, // mip id
					                        cubeface, // base array layer
					                        1); // array count
				}

				// calculate metrics of the next mip
				ut::uint32 mip_size = pixel_size * mip_width;
				mip_width /= 2;

				if (is_2d || is_3d)
				{
					mip_size *= mip_height;
					mip_height /= 2;
				}

				if (is_3d)
				{
					mip_size *= mip_depth;
					mip_depth /= 2;
				}

				mip_data += mip_size;
			}
		}

		// unmap image memory
		vkUnmapMemory(device.GetVkHandle(), mapped_memory);
	}

	// set final layout
	VkImageMemoryBarrier img_barrier;
	img_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	img_barrier.pNext = nullptr;
	img_barrier.srcAccessMask = current_img_access;
	img_barrier.dstAccessMask = image_state.access_mask;
	img_barrier.oldLayout = image_info.initialLayout;
	img_barrier.newLayout = image_state.layout;
	img_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	img_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	img_barrier.image = image;
	img_barrier.subresourceRange.aspectMask = aspect_mask;
	img_barrier.subresourceRange.baseMipLevel = 0;
	img_barrier.subresourceRange.levelCount = image_info.mipLevels;
	img_barrier.subresourceRange.baseArrayLayer = 0;
	img_barrier.subresourceRange.layerCount = image_info.arrayLayers;
	vkCmdPipelineBarrier(immediate_buffer.Get(),
	                     needs_staging ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_HOST_BIT,
	                     image_state.stages,
	                     0, 0, nullptr, 0, nullptr,
	                     1, &img_barrier);

	// submit all gpu commands
	ut::Optional<ut::Error> end_cmd_buffer_error = EndImmediateCmdBuffer(immediate_buffer.Get());
	if (end_cmd_buffer_error)
	{
		return ut::MakeError(end_cmd_buffer_error.Move());
	}

	// shader can't read depth and stencil simultaneously
	VkImageAspectFlags view_aspect_mask = aspect_mask & ~VK_IMAGE_ASPECT_STENCIL_BIT;

	// initialize image view info
	VkImageViewCreateInfo view_info;
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = nullptr;
	view_info.flags = 0;
	view_info.image = image;
	view_info.viewType = ConvertImageViewTypeToVulkan(info.type);
	view_info.format = image_info.format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = view_aspect_mask;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = image_info.mipLevels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = image_info.arrayLayers;

	// create image view
	VkImageView image_view;
	res = vkCreateImageView(device.GetVkHandle(), &view_info, nullptr, &image_view);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateImageView"));
	}

	// image views for all cube faces
	VkImageView cube_faces[6];
	if (is_cube)
	{
		for (ut::uint32 cube_face = 0; cube_face < 6; cube_face++)
		{
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.pNext = nullptr;
			view_info.flags = 0;
			view_info.image = image;
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = image_info.format;
			view_info.components.r = VK_COMPONENT_SWIZZLE_R;
			view_info.components.g = VK_COMPONENT_SWIZZLE_G;
			view_info.components.b = VK_COMPONENT_SWIZZLE_B;
			view_info.components.a = VK_COMPONENT_SWIZZLE_A;
			view_info.subresourceRange.aspectMask = view_aspect_mask;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = image_info.mipLevels;
			view_info.subresourceRange.baseArrayLayer = cube_face;
			view_info.subresourceRange.layerCount = 1;

			res = vkCreateImageView(device.GetVkHandle(), &view_info, nullptr, &cube_faces[cube_face]);
			if (res != VK_SUCCESS)
			{
				return ut::MakeError(VulkanError(res, "vkCreateImageView(cube face)"));
			}
		}
	}

	// success
	PlatformImage platform_img(device.GetVkHandle(),
	                           image,
	                           image_view,
	                           is_cube ? cube_faces : nullptr,
	                           ut::Move(image_memory),
	                           aspect_mask,
	                           image_state);
	return Image(ut::Move(platform_img), info);
}

// Creates a new sampler.
//    @param info - reference to the Sampler::Info object describing a sampler.
//    @return - new sampler object of error if failed.
ut::Result<Sampler, ut::Error> Device::CreateSampler(const Sampler::Info& info)
{
	VkSamplerCreateInfo sampler_info;
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.pNext = nullptr;
	sampler_info.flags = 0;
	sampler_info.magFilter = ConvertFilterToVulkan(info.mag_filter);
	sampler_info.minFilter = ConvertFilterToVulkan(info.min_filter);
	sampler_info.mipmapMode = ConvertMipmapModeToVulkan(info.mip_filter);
	sampler_info.addressModeU = ConvertAddressModeToVulkan(info.address_u);
	sampler_info.addressModeV = ConvertAddressModeToVulkan(info.address_v);
	sampler_info.addressModeW = ConvertAddressModeToVulkan(info.address_w);
	sampler_info.mipLodBias = info.mip_lod_bias;
	sampler_info.anisotropyEnable = info.anisotropy_enable ? VK_TRUE : VK_FALSE;
	sampler_info.maxAnisotropy = info.max_anisotropy;
	sampler_info.compareEnable = info.compare_op ? VK_TRUE : VK_FALSE;
	sampler_info.compareOp = info.compare_op ? ConvertCompareOpToVulkan(info.compare_op.Get()) : VK_COMPARE_OP_ALWAYS;
	sampler_info.minLod = info.min_lod;
	sampler_info.maxLod = info.max_lod;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;

	VkSampler sampler;
	VkResult res = vkCreateSampler(device.GetVkHandle(), &sampler_info, nullptr, &sampler);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateSampler"));
	}

	return Sampler(PlatformSampler(device.GetVkHandle(), sampler), info);
}

// Creates a new render target.
//    @param info - reference to the Target::Info object describing a target.
//    @return - new render target object of error if failed.
ut::Result<Target, ut::Error> Device::CreateTarget(const Target::Info& info)
{
	// create image resource for the render target
	Image::Info img_info;
	img_info.type = info.type;
	img_info.format = info.format;
	img_info.usage = render::memory::gpu_read_write;
	img_info.mip_count = info.mip_count;
	img_info.width = info.width;
	img_info.height = info.height;
	img_info.depth = info.depth;
	ut::Result<Image, ut::Error> image = CreateImage(img_info);
	if (!image)
	{
		return ut::MakeError(image.MoveAlt());
	}

	// target is always created in 'target' state
	Target::Info target_info(info);
	target_info.state = Target::state_target;

	return Target(PlatformRenderTarget(), image.Move(), info);
}

// Creates platform-specific representation of the rendering area inside a UI viewport.
//    @param viewport - reference to UI viewport containing rendering area.
//    @param vsync - boolean whether to enable vertical synchronization or not.
//    @return - new display object or error if failed.
ut::Result<Display, ut::Error> Device::CreateDisplay(ui::PlatformViewport& viewport, bool vsync)
{
	// surface of the display
	VkSurfaceKHR surface;

#if UT_WINDOWS
	// extract windows handle from the viewport widget
	const HWND hwnd = fl_xid(&viewport);
	HINSTANCE hinstance = fl_display;

	// fill info stuct object before creating surface
	VkWin32SurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.hinstance = fl_display;
	create_info.hwnd = hwnd;

	// create surface
	VkResult res = vkCreateWin32SurfaceKHR(instance.GetVkHandle(), &create_info, nullptr, &surface);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateWin32SurfaceKHR"));
	}
#elif UT_LINUX && VE_X11
	// fill info stuct object before creating surface
	VkXlibSurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.flags = 0;
	create_info.dpy = fl_display;
	create_info.window = fl_xid(&viewport);

	// create surface
	VkResult res = vkCreateXlibSurfaceKHR(instance.GetVkHandle(), &create_info, nullptr, &surface);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateXlibSurfaceKHR"));
	}
#endif
	
	// check if main queue supports present
	VkBool32 supports_present;
	res = vkGetPhysicalDeviceSurfaceSupportKHR(gpu, main_queue.GetDetail().GetQueueFamilyId(), surface, &supports_present);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceSupportKHR"));
	}

	// if present is not supported - there is no way to display an image to user
	if (!supports_present)
	{
		return ut::MakeError(ut::Error(ut::error::not_supported, "Vulkan: present feauture is not supported."));
	}

	// get the number of VkFormats that are supported by surface
	uint32_t format_count;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, nullptr);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	}

	// get the list of VkFormats that are supported
	ut::Array<VkSurfaceFormatKHR> surf_formats(format_count);
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, surf_formats.GetAddress());
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	}

	// validate format count, it must have at least one value
	if (format_count == 0)
	{
		return ut::MakeError(ut::Error(ut::error::empty, "Vulkan: empty format list for given surface."));
	}

	// if the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format; otherwise, at least one
	// supported format will be returned
	VkFormat surface_format;
	if (format_count == 1 && surf_formats[0].format == VK_FORMAT_UNDEFINED)
	{
		surface_format = VK_FORMAT_R8G8B8A8_SRGB;
	}
	else
	{
		// search if srgb is supported
		bool found_srgb = false;
		for (uint32_t i = 0; i < format_count; i++)
		{
			const VkFormat supported_format = surf_formats[i].format;
			if (supported_format == VK_FORMAT_R8G8B8A8_SRGB ||
				supported_format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				surface_format = supported_format;
				found_srgb = true;
				break;
			}
		}

		// otherwise use first supported
		if (!found_srgb)
		{
			surface_format = surf_formats[0].format;
		}
	}
	
	// surface capabilities
	VkSurfaceCapabilitiesKHR surf_capabilities;
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surf_capabilities);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	}

	// get the number of present modes
	uint32_t present_mode_count;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, nullptr);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfacePresentModesKHR(count)"));
	}

	// get the list of present modes
	ut::Array<VkPresentModeKHR> present_modes(present_mode_count);
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, present_modes.GetAddress());
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
	}
	
	// get size of the viewport
	const ut::uint32 width = static_cast<ut::uint32>(viewport.w());
	const ut::uint32 height = static_cast<ut::uint32>(viewport.h());

	// width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
	VkExtent2D swapchain_extent;
	if (surf_capabilities.currentExtent.width == 0xFFFFFFFF)
	{
		swapchain_extent.width = ut::Clamp(width,
		                                   surf_capabilities.minImageExtent.width,
		                                   surf_capabilities.maxImageExtent.width);
		swapchain_extent.height = ut::Clamp(height,
		                                    surf_capabilities.minImageExtent.height,
		                                    surf_capabilities.maxImageExtent.height);
	}
	else
	{
		// if the surface size is defined, the swap chain size must match
		swapchain_extent = surf_capabilities.currentExtent;
	}

	// the FIFO present mode is guaranteed by the spec to be supported
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

	// check if immediate mode is supported
	if (!vsync)
	{
		// switch back to vsync enabled state until it's known
		// that immediate mode is supported
		vsync = true;

		// iterate all modes to find immediate one
		for (uint32_t i = 0; i < present_mode_count; i++)
		{
			if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR || 
			    present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				present_mode = present_modes[i];
				vsync = false;
				break;
			}
		}
	}

	// Determine the number of VkImage's to use in the swap chain.
	// We need to acquire only 1 presentable image at at time.
	// Asking for minImageCount images ensures that we can acquire
	// 1 presentable image as long as we present it before attempting
	// to acquire another.
	uint32_t desired_number_of_swap_chain_images = surf_capabilities.minImageCount;

	// surface transform
	VkSurfaceTransformFlagBitsKHR pre_transform;
	if (surf_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		pre_transform = surf_capabilities.currentTransform;
	}

	// Find a supported composite alpha mode - one of these is guaranteed to be set
	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] =
	{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (uint32_t i = 0; i < sizeof(composite_alpha_flags) / sizeof(composite_alpha_flags[0]); i++)
	{
		if (surf_capabilities.supportedCompositeAlpha & composite_alpha_flags[i])
		{
			composite_alpha = composite_alpha_flags[i];
			break;
		}
	}

	// fill swapchain info object
	VkSwapchainCreateInfoKHR swapchain_ci = {};
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_ci.pNext = nullptr;
	swapchain_ci.surface = surface;
	swapchain_ci.minImageCount = desired_number_of_swap_chain_images;
	swapchain_ci.imageFormat = surface_format;
	swapchain_ci.imageExtent.width = swapchain_extent.width;
	swapchain_ci.imageExtent.height = swapchain_extent.height;
	swapchain_ci.preTransform = pre_transform;
	swapchain_ci.compositeAlpha = composite_alpha;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = present_mode;
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
	swapchain_ci.clipped = true;
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = nullptr;

	// create swapchain for this viewport
	VkSwapchainKHR swap_chain;
	res = vkCreateSwapchainKHR(device.GetVkHandle(), &swapchain_ci, nullptr, &swap_chain);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateSwapchainKHR"));
	}

	// get buffer count
	uint32_t image_count;
	res = vkGetSwapchainImagesKHR(device.GetVkHandle(), swap_chain, &image_count, nullptr);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetSwapchainImagesKHR(count)"));
	}

	// get back and front buffers
	ut::Array<VkImage> swapchain_images(image_count);
	res = vkGetSwapchainImagesKHR(device.GetVkHandle(), swap_chain, &image_count, swapchain_images.GetAddress());
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetSwapchainImagesKHR"));
	}

	// create render targets for all buffers in a swapchain
	ut::Array<Target> targets;
	for (uint32_t i = 0; i < image_count; i++)
	{
		// initialize image view info
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = nullptr;
		color_image_view.flags = 0;
		color_image_view.image = swapchain_images[i];
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.format = surface_format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;

		// create image view for the current buffer
		VkImageView view;
		res = vkCreateImageView(device.GetVkHandle(), &color_image_view, nullptr, &view);
		if (res != VK_SUCCESS)
		{
			return ut::MakeError(VulkanError(res, "vkCreateImageView(swapchain)"));
		}

		// stub texture
		Image::Info info;
		switch (surface_format)
		{
		case VK_FORMAT_R8G8B8A8_SRGB: info.format = pixel::r8g8b8a8_srgb; break;
		case VK_FORMAT_B8G8R8A8_SRGB: info.format = pixel::b8g8r8a8_srgb; break;
		case VK_FORMAT_R8G8B8A8_UNORM: info.format = pixel::r8g8b8a8_unorm; break;
		case VK_FORMAT_B8G8R8A8_UNORM: info.format = pixel::b8g8r8a8_unorm; break;
		default: return ut::MakeError(VulkanError(res, "Unsupported surface format."));
		}
		info.width = swapchain_extent.width;
		info.height = swapchain_extent.height;
		info.depth = 1;

		PlatformImage::State img_state(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, VK_PIPELINE_STAGE_HOST_BIT);
		PlatformImage empty_img(device.GetVkHandle(),
		                        VK_NULL_HANDLE, // handle
		                        view, // view
			                    nullptr, // cube faces
		                        VK_NULL_HANDLE, // memory
		                        VK_IMAGE_ASPECT_COLOR_BIT, // aspect_mask
		                        img_state);
		Image image(ut::Move(empty_img), info);

		// initialize render target info
		Target::Info target_info;
		target_info.usage = Target::usage_present;

		// create final target for the current buffer
		Target target(PlatformRenderTarget(), ut::Move(image), target_info);
		if (!targets.Add(ut::Move(target)))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// create platform-specific display object
	PlatformDisplay platform_display(instance.GetVkHandle(), device.GetVkHandle(), surface, swap_chain, image_count);

	// create final display object
	Display out_display(ut::Move(platform_display), ut::Move(targets), width, height, vsync);

	// success
	return out_display;
}

// Creates an empty command buffer.
//    @param cmd_buffer_info - reference to the information about
//                             the command buffer to be created.
ut::Result<CmdBuffer, ut::Error> Device::CreateCmdBuffer(const CmdBuffer::Info& cmd_buffer_info)
{
	bool is_dynamic = (cmd_buffer_info.usage & CmdBuffer::usage_dynamic) ? true : false;
	bool is_primary = cmd_buffer_info.level == CmdBuffer::level_primary;

	ut::Result<VkCommandPool, ut::Error> cmd_pool = CreateCmdPool(is_dynamic ?
	                                                              VK_COMMAND_POOL_CREATE_TRANSIENT_BIT :
	                                                              VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	if (!cmd_pool)
	{
		return ut::MakeError(cmd_pool.MoveAlt());
	}

	VkCommandBufferAllocateInfo cmd_info = {};
	cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_info.pNext = nullptr;
	cmd_info.commandPool = cmd_pool.Get();
	cmd_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	cmd_info.commandBufferCount = 1;

	VkCommandBuffer cmd_buffer;
	VkResult res = vkAllocateCommandBuffers(device.GetVkHandle(), &cmd_info, &cmd_buffer);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkAllocateCommandBuffers"));
	}

	VkFenceCreateInfo fence_info;
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = 0;

	VkFence fence;
	res = vkCreateFence(device.GetVkHandle(), &fence_info, nullptr, &fence);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateFence(cmdbuffer)"));
	}

	PlatformCmdBuffer platform_buffer(device.GetVkHandle(), cmd_pool.Get(), cmd_buffer, fence);
	return CmdBuffer(ut::Move(platform_buffer), cmd_buffer_info);
}

// Creates render pass object.
//    @param in_color_slots - array of color slots.
//    @param in_depth_stencil_slot - optional depth stencil slot.
//    @return - new render pass or error if failed.
ut::Result<RenderPass, ut::Error> Device::CreateRenderPass(ut::Array<RenderTargetSlot> in_color_slots,
	                                                       ut::Optional<RenderTargetSlot> in_depth_stencil_slot)
{
	ut::Array<VkAttachmentDescription> attachments;

	// initialize color attachments
	bool has_present_surface = false;
	ut::Array<VkAttachmentReference> color_references;
	for (size_t i = 0; i < in_color_slots.GetNum(); i++)
	{
		VkAttachmentDescription color_attachment;

		// convert pixel format
		color_attachment.format = ConvertPixelFormatToVulkan(in_color_slots[i].format);

		// always 1 sample
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		// load/store operations
		color_attachment.loadOp = ConvertLoadOpToVulkan(in_color_slots[i].load_op);
		color_attachment.storeOp = ConvertStoreOpToVulkan(in_color_slots[i].store_op);

		// no stencil operations
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// initial layout
		if (in_color_slots[i].load_op == RenderTargetSlot::load_clear)
		{
			color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		// final layout
		if (in_color_slots[i].present_surface)
		{
			color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			has_present_surface = true;
		}
		else
		{
			color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		// no flags
		color_attachment.flags = 0;

		// add attachment to the array
		if (!attachments.Add(color_attachment))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}

		// create reference
		VkAttachmentReference color_reference = {};
		color_reference.attachment = static_cast<uint32_t>(i);
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// add reference to the array
		if (!color_references.Add(color_reference))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// initialize depth-stencil attachment
	VkAttachmentReference depth_reference = {};
	if (in_depth_stencil_slot)
	{
		RenderTargetSlot& depth_stencil_slot = in_depth_stencil_slot.Get();

		VkAttachmentDescription depth_attachment;

		// convert pixel format
		depth_attachment.format = ConvertPixelFormatToVulkan(depth_stencil_slot.format);

		// always 1 sample
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		// load/store operations
		depth_attachment.loadOp = ConvertLoadOpToVulkan(depth_stencil_slot.load_op);
		depth_attachment.storeOp = ConvertStoreOpToVulkan(depth_stencil_slot.store_op);
		depth_attachment.stencilLoadOp = ConvertLoadOpToVulkan(depth_stencil_slot.load_op);
		depth_attachment.stencilStoreOp = ConvertStoreOpToVulkan(depth_stencil_slot.store_op);

		// initial layout
		if (depth_stencil_slot.load_op == RenderTargetSlot::load_clear)
		{
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		// final layout
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// no flags
		depth_attachment.flags = 0;

		// add to the array
		if (!attachments.Add(depth_attachment))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}

		// initialize reference
		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = static_cast<uint32_t>(color_references.GetNum());
	subpass.pColorAttachments = color_references.GetAddress();
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = in_depth_stencil_slot ? &depth_reference : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	// subpass dependency to wait for wsi image acquired
	// semaphore before starting layout transition
	VkSubpassDependency subpass_dependency = {};
	subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.srcAccessMask = 0;
	subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependency.dependencyFlags = 0;

	// add depth stencil access flags
	if (in_depth_stencil_slot)
	{
		subpass_dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpass_dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		subpass_dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	// initialize render pass info
	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = nullptr;
	rp_info.attachmentCount = static_cast<uint32_t>(attachments.GetNum());
	rp_info.pAttachments = attachments.GetAddress();
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = has_present_surface ? 1 : 0;
	rp_info.pDependencies = has_present_surface ? &subpass_dependency : nullptr;

	// create render pass
	VkRenderPass render_pass;
	VkResult res = vkCreateRenderPass(device.GetVkHandle(), &rp_info, nullptr, &render_pass);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateRenderPass"));
	}

	// success
	PlatformRenderPass platform_render_pass(device.GetVkHandle(), render_pass);
	return RenderPass(ut::Move(platform_render_pass), ut::Move(in_color_slots), ut::Move(in_depth_stencil_slot));
}

// Creates a framebuffer. All targets must have the same width and height.
//    @param render_pass - const reference to the renderpass to be bound to.
//    @param color_targets - array of references to the colored render
//                           targets to be bound to a render pass.
//    @param depth_stencil_target - optional reference to the depth-stencil
//                                  target to be bound to a render pass.
//    @return - new framebuffer or error if failed.
ut::Result<Framebuffer, ut::Error> Device::CreateFramebuffer(const RenderPass& render_pass,
	                                                         ut::Array< ut::Ref<Target> > color_targets,
	                                                         ut::Optional<Target&> depth_stencil_target)
{
	// determine width and heights of the framebuffer in pixels
	ut::uint32 width;
	ut::uint32 height;
	if (color_targets.GetNum() != 0)
	{
		const Target& color_target = color_targets.GetFirst();
		width = color_target.data->image.GetInfo().width;
		height = color_target.data->image.GetInfo().height;
	}
	else if (depth_stencil_target)
	{
		const Target& ds_target = depth_stencil_target.Get();
		width = ds_target.data->image.GetInfo().width;
		height = ds_target.data->image.GetInfo().height;
	}
	else
	{
		return ut::MakeError(ut::Error(ut::error::invalid_arg, "Vulkan: no targets for framebuffer."));
	}

	// color attachments
	ut::Array<VkImageView> images;
	const size_t color_target_count = color_targets.GetNum();
	for (size_t i = 0; i < color_target_count; i++)
	{
		const Target& color_target = color_targets[i];

		// check width and height
		const Image::Info& img_info = color_target.data->image.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "Vulkan: different width/height for framebuffer."));
		}

		// add attachment
		if (!images.Add(color_target.data->image.view.GetVkHandle()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// depth attachment
	if (depth_stencil_target)
	{
		const Target& ds_target = depth_stencil_target.Get();

		// check width and height
		const Image::Info& img_info = ds_target.data->image.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "Vulkan: different width/height for framebuffer."));
		}

		// add attachment
		if (!images.Add(ds_target.data->image.view.GetVkHandle()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// vulkan framebuffer info
	VkFramebufferCreateInfo vk_fb_info = {};
	vk_fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	vk_fb_info.pNext = nullptr;
	vk_fb_info.renderPass = render_pass.GetVkHandle();
	vk_fb_info.attachmentCount = static_cast<uint32_t>(images.GetNum());
	vk_fb_info.pAttachments = images.GetAddress();
	vk_fb_info.width = static_cast<uint32_t>(width);
	vk_fb_info.height = static_cast<uint32_t>(height);
	vk_fb_info.layers = 1;

	// create framebuffer
	VkFramebuffer framebuffer;
	VkResult res = vkCreateFramebuffer(device.GetVkHandle(), &vk_fb_info, nullptr, &framebuffer);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateFramebuffer"));
	}

	// initialize framebuffer info
	Framebuffer::Info framebuffer_info;
	framebuffer_info.width = width;
	framebuffer_info.height = height;

	// colored shared target data
	ut::Array<Target::SharedData> color_shared_data;
	for (size_t i = 0; i < color_target_count; i++)
	{
		color_shared_data.Add(color_targets[i]->data);
	}

	// colored depth stencil target data
	ut::Optional<Target::SharedData> depth_stencil_shared_data;
	if (depth_stencil_target)
	{
		depth_stencil_shared_data = depth_stencil_target->data;
	}

	// success
	PlatformFramebuffer platform_framebuffer(device.GetVkHandle(),
	                                         framebuffer);
	return Framebuffer(ut::Move(platform_framebuffer),
	                   framebuffer_info,
	                   ut::Move(color_shared_data),
	                   ut::Move(depth_stencil_shared_data));
}

// Creates a buffer.
//    @param info - ve::render::Buffer::Info object to initialize a buffer with.
//    @return - new shader or error if failed.
ut::Result<Buffer, ut::Error> Device::CreateBuffer(Buffer::Info info)
{
	// validate data size
	size_t ini_size = info.data.GetSize();
	if (ini_size > info.size)
	{
		return ut::MakeError(ut::Error(ut::error::invalid_arg, "Initialization data is bigger than buffer size."));
	}

	// find valid usage for the buffer
	int usage = ConvertBufferTypeToVulkan(info.type);
	if (info.usage == render::memory::gpu_immutable)
	{
		usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	// create buffer
	ut::Result<VkBuffer, ut::Error> buffer_result = CreateVulkanBuffer(info.size, usage);
	if (!buffer_result)
	{
		return ut::MakeError(buffer_result.MoveAlt());
	}
	VkBuffer buffer = buffer_result.Get();

	// allocate memory
	VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	if (info.usage == render::memory::gpu_immutable)
	{
		memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	ut::Result<VkRc<vk::memory>, ut::Error> memory_result = AllocateBufferMemory(buffer, memory_properties);
	if (!memory_result)
	{
		return ut::MakeError(memory_result.MoveAlt());
	}
	VkRc<vk::memory> buffer_memory(memory_result.Move());

	// copy memory
	if (ini_size > 0)
	{
		if (info.usage == render::memory::gpu_immutable)
		{
			// create staging buffer
			ut::Result<PlatformBuffer, ut::Error> staging_buffer = CreateStagingBuffer(info.size, info.data);
			if (!staging_buffer)
			{
				return ut::MakeError(staging_buffer.MoveAlt());
			}

			// start immediate commands
			ut::Result<VkCommandBuffer, ut::Error> immediate_buffer = BeginImmediateCmdBuffer();
			if (!immediate_buffer)
			{
				return ut::MakeError(immediate_buffer.MoveAlt());
			}

			// record copy command
			CopyVulkanBuffer(immediate_buffer.Get(), staging_buffer->GetVkHandle(), buffer, ini_size);

			// submit copying
			ut::Optional<ut::Error> end_cmd_buffer_error = EndImmediateCmdBuffer(immediate_buffer.Get());
			if (end_cmd_buffer_error)
			{
				return ut::MakeError(end_cmd_buffer_error.Move());
			}
		}
		else
		{
			// copy memory directly to the buffer
			void* buffer_data;
			VkResult res = vkMapMemory(device.GetVkHandle(), buffer_memory.GetVkHandle(), 0, ini_size, 0, &buffer_data);
			if (res != VK_SUCCESS)
			{
				return ut::MakeError(VulkanError(res, "vkMapMemory(initializing buffer)"));
			}
			ut::memory::Copy(buffer_data, info.data.GetAddress(), info.data.GetSize());
			vkUnmapMemory(device.GetVkHandle(), buffer_memory.GetVkHandle());
		}
	}

	// success
	PlatformBuffer platform_buffer(device.GetVkHandle(), buffer, ut::Move(buffer_memory));
	return Buffer(ut::Move(platform_buffer), ut::Move(info));
}

// Creates a shader.
//    @param info - ve::Shader::Info object to initialize a shader with.
//    @return - new shader or error if failed.
ut::Result<Shader, ut::Error> Device::CreateShader(Shader::Info info)
{
	// create shader module
	VkShaderModule shader_module;
	VkShaderModuleCreateInfo module_info;
	module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_info.pNext = nullptr;
	module_info.flags = 0;
	module_info.codeSize = info.bytecode.GetSize();
	module_info.pCode = reinterpret_cast<const uint32_t*>(info.bytecode.GetAddress());
	VkResult res = vkCreateShaderModule(device.GetVkHandle(), &module_info, nullptr, &shader_module);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateShaderModule"));
	}

	// create shader stage
	VkPipelineShaderStageCreateInfo shader_stage;
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.pNext = nullptr;
	shader_stage.pSpecializationInfo = nullptr;
	shader_stage.flags = 0;
	shader_stage.stage = PlatformShader::ConvertTypeToVkStage(info.stage);
	shader_stage.module = shader_module;
	shader_stage.pName = info.entry_point.ToCStr();

	// success
	PlatformShader platform_shader(device.GetVkHandle(), shader_module, shader_stage);
	return Shader(ut::Move(platform_shader), ut::Move(info));
}

// Creates a pipeline state.
//    @param info - ve::render::PipelineState::Info object to
//                  initialize a pipeline with.
//    @param render_pass - reference to the ve::render::RenderPass object
//                         pipeline will be bound to.
//    @return - new pipeline sate or error if failed.
ut::Result<PipelineState, ut::Error> Device::CreatePipelineState(PipelineState::Info info, RenderPass& render_pass)
{
	// form shader array
	ut::Array< ut::Ref<Shader> > shaders;
	for (size_t i = 0; i < Shader::skStageCount; i++)
	{
		if (info.stages[i])
		{
			shaders.Add(info.stages[i].Get());
		}
	}

	// shader stages
	const uint32_t shader_stage_count = static_cast<uint32_t>(shaders.GetNum());
	ut::Array<VkPipelineShaderStageCreateInfo> shader_stages;
	for (uint32_t i = 0; i < shader_stage_count; i++)
	{
		if (!shader_stages.Add(shaders[i]->stage_info))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// vertex binding
	VkVertexInputBindingDescription vertex_binding;
	vertex_binding.binding = 0;
	vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertex_binding.stride = info.input_assembly_state.stride;

	// vertex attributes
	ut::Array<VkVertexInputAttributeDescription> vertex_attributes;
	for (size_t i = 0; i < info.input_assembly_state.elements.GetNum(); i++)
	{
		VertexElement& element = info.input_assembly_state.elements[i];

		VkVertexInputAttributeDescription attribute_desc;
		attribute_desc.binding = 0;
		attribute_desc.location = static_cast<uint32_t>(i);
		attribute_desc.format = ConvertPixelFormatToVulkan(element.format);
		attribute_desc.offset = element.offset;

		if (!vertex_attributes.Add(attribute_desc))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// vertex input state
	VkPipelineVertexInputStateCreateInfo vertex_input;
	ut::memory::Set(&vertex_input, 0, sizeof(vertex_input));
	vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input.pNext = nullptr;
	vertex_input.flags = 0;
	vertex_input.vertexBindingDescriptionCount = 1;
	vertex_input.pVertexBindingDescriptions = &vertex_binding;
	vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes.GetNum());
	vertex_input.pVertexAttributeDescriptions = vertex_attributes.GetAddress();

	// input assembly state
	VkPipelineInputAssemblyStateCreateInfo input_assembly;
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.pNext = nullptr;
	input_assembly.flags = 0;
	input_assembly.primitiveRestartEnable = VK_FALSE;
	input_assembly.topology = ConvertPrimitiveTopologyToVulkan(info.input_assembly_state.topology);
	if (input_assembly.topology == VK_PRIMITIVE_TOPOLOGY_MAX_ENUM)
	{
		return ut::MakeError(ut::Error(ut::error::invalid_arg, "Primitive topology can't be converted to Vulkan"));
	}

	// generate array of vulkan viewports
	uint32_t viewport_count = static_cast<uint32_t>(info.viewports.GetNum());
	ut::Array<VkViewport> viewports(viewport_count);
	ut::Array<VkRect2D> scissors(viewport_count);
	for (uint32_t i = 0; i < viewport_count; i++)
	{
		// note that height is inverted to make Y up
		viewports[i].x = info.viewports[i].x;
		viewports[i].y = info.viewports[i].buffer_height - info.viewports[i].y;
		viewports[i].width = info.viewports[i].width;
		viewports[i].height = -info.viewports[i].height;
		viewports[i].minDepth = info.viewports[i].min_depth;
		viewports[i].maxDepth = info.viewports[i].max_depth;

		if (info.viewports[i].scissor)
		{
			scissors[i].offset.x = info.viewports[i].scissor->offset.X();
			scissors[i].offset.y = info.viewports[i].scissor->offset.Y();
			scissors[i].extent.width = info.viewports[i].scissor->extent.X();
			scissors[i].extent.height = info.viewports[i].scissor->extent.Y();
		}
		else
		{
			scissors[i].offset.x = static_cast<int32_t>(info.viewports[i].x);
			scissors[i].offset.y = static_cast<int32_t>(info.viewports[i].y);
			scissors[i].extent.width = static_cast<uint32_t>(info.viewports[i].width);
			scissors[i].extent.height = static_cast<uint32_t>(info.viewports[i].height);
		}

		int32_t scissor_bottom = static_cast<int32_t>(info.viewports[i].buffer_height) - scissors[i].offset.y;
		scissors[i].offset.y = scissor_bottom - scissors[i].extent.height;
	}

	// viewport state
	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.pNext = nullptr;
	viewport_state.flags = 0;
	viewport_state.viewportCount = viewport_count;
	viewport_state.scissorCount = viewport_count;
	viewport_state.pViewports = viewports.GetAddress();
	viewport_state.pScissors = scissors.GetAddress();

	// rasterization state
	VkPipelineRasterizationStateCreateInfo rasterization_state;
	rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state.pNext = nullptr;
	rasterization_state.flags = 0;
	rasterization_state.polygonMode = ConvertPolygonModeToVulkan(info.rasterization_state.polygon_mode);
	rasterization_state.cullMode = ConvertCullModeToVulkan(info.rasterization_state.cull_mode);
	rasterization_state.frontFace = ConvertFrontFaceToVulkan(info.rasterization_state.front_face);
	rasterization_state.depthClampEnable = VK_FALSE;
	rasterization_state.rasterizerDiscardEnable = info.rasterization_state.discard_enable ? VK_TRUE : VK_FALSE;
	rasterization_state.depthBiasEnable = info.rasterization_state.depth_bias_enable ? VK_TRUE : VK_FALSE;
	rasterization_state.depthBiasConstantFactor = info.rasterization_state.depth_bias_constant_factor;
	rasterization_state.depthBiasClamp = info.rasterization_state.depth_bias_clamp;
	rasterization_state.depthBiasSlopeFactor = info.rasterization_state.depth_bias_slope_factor;
	rasterization_state.lineWidth = info.rasterization_state.line_width;

	// multisampling state
	VkPipelineMultisampleStateCreateInfo multisampling;
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.pNext = nullptr;
	multisampling.flags = 0;
	multisampling.pSampleMask = nullptr;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;
	multisampling.minSampleShading = 0.0;

	// depth-stencil state
	VkPipelineDepthStencilStateCreateInfo ds_state;
	ds_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ds_state.pNext = NULL;
	ds_state.flags = 0;
	ds_state.depthTestEnable = info.depth_stencil_state.depth_test_enable ? VK_TRUE : VK_FALSE;
	ds_state.depthWriteEnable = info.depth_stencil_state.depth_write_enable ? VK_TRUE : VK_FALSE;
	ds_state.depthCompareOp = ConvertCompareOpToVulkan(info.depth_stencil_state.depth_compare_op);
	ds_state.stencilTestEnable = info.depth_stencil_state.stencil_test_enable ? VK_TRUE : VK_FALSE;
	ds_state.back.failOp = ConvertStencilOpToVulkan(info.depth_stencil_state.back.fail_op);
	ds_state.back.passOp = ConvertStencilOpToVulkan(info.depth_stencil_state.back.pass_op);
	ds_state.back.depthFailOp = ConvertStencilOpToVulkan(info.depth_stencil_state.back.depth_fail_op);
	ds_state.back.compareOp = ConvertCompareOpToVulkan(info.depth_stencil_state.back.compare_op);
	ds_state.back.compareMask = info.depth_stencil_state.back.compare_mask;
	ds_state.back.reference = info.depth_stencil_state.stencil_reference;
	ds_state.back.writeMask = info.depth_stencil_state.stencil_write_mask;
	ds_state.front.failOp = ConvertStencilOpToVulkan(info.depth_stencil_state.front.fail_op);
	ds_state.front.passOp = ConvertStencilOpToVulkan(info.depth_stencil_state.front.pass_op);
	ds_state.front.depthFailOp = ConvertStencilOpToVulkan(info.depth_stencil_state.front.depth_fail_op);
	ds_state.front.compareOp = ConvertCompareOpToVulkan(info.depth_stencil_state.front.compare_op);
	ds_state.front.compareMask = info.depth_stencil_state.front.compare_mask;
	ds_state.front.reference = info.depth_stencil_state.stencil_reference;
	ds_state.front.writeMask = info.depth_stencil_state.stencil_write_mask;
	ds_state.depthBoundsTestEnable = VK_FALSE;
	ds_state.minDepthBounds = 0.0f;
	ds_state.maxDepthBounds = 1.0f;

	// blend attachments
	const uint32_t blend_attachment_count = static_cast<uint32_t>(info.blend_state.attachments.GetNum());
	ut::Array<VkPipelineColorBlendAttachmentState> blend_attachments;
	for (uint32_t i = 0; i < blend_attachment_count; i++)
	{
		Blending& attachment = info.blend_state.attachments[i];
		VkPipelineColorBlendAttachmentState att_state;
		att_state.colorWriteMask = attachment.write_mask;
		att_state.blendEnable = attachment.blend_enable ? VK_TRUE : VK_FALSE;
		att_state.alphaBlendOp = ConvertBlendOpToVulkan(attachment.alpha_op);
		att_state.colorBlendOp = ConvertBlendOpToVulkan(attachment.color_op);
		att_state.srcColorBlendFactor = ConvertBlendFactorToVulkan(attachment.src_blend);
		att_state.dstColorBlendFactor = ConvertBlendFactorToVulkan(attachment.dst_blend);
		att_state.srcAlphaBlendFactor = ConvertBlendFactorToVulkan(attachment.src_blend_alpha);
		att_state.dstAlphaBlendFactor = ConvertBlendFactorToVulkan(attachment.dst_blend_alpha);

		if (!blend_attachments.Add(att_state))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// blend state
	VkPipelineColorBlendStateCreateInfo blend_state;
	blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend_state.flags = 0;
	blend_state.pNext = nullptr;
	blend_state.attachmentCount = static_cast<uint32_t>(blend_attachments.GetNum());
	blend_state.pAttachments = blend_attachments.GetAddress();
	blend_state.logicOpEnable = VK_FALSE;
	blend_state.logicOp = VK_LOGIC_OP_NO_OP;
	blend_state.blendConstants[0] = 1.0f;
	blend_state.blendConstants[1] = 1.0f;
	blend_state.blendConstants[2] = 1.0f;
	blend_state.blendConstants[3] = 1.0f;

	// tessellation state
	VkPipelineTessellationStateCreateInfo tessellation_state;
	tessellation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	tessellation_state.pNext = nullptr;
	tessellation_state.flags = 0;
	tessellation_state.patchControlPoints = 3; // triangle

	// create bindings for all uniforms in every shader stage
	ut::Array<VkDescriptorSetLayoutBinding> layout_bindings;
	for (uint32_t stage_id = 0; stage_id < shader_stage_count; stage_id++)
	{
		Shader& shader = shaders[stage_id];
		for (size_t param_id = 0; param_id < shader.info.parameters.GetNum(); param_id++)
		{
			Shader::Parameter& parameter = shader.info.parameters[param_id];

			VkDescriptorSetLayoutBinding dslb;
			dslb.binding = parameter.GetBinding();
			dslb.descriptorCount = parameter.GetElementCount();
			dslb.pImmutableSamplers = nullptr;
			dslb.stageFlags = PlatformShader::ConvertTypeToVkStage(shader.info.stage);
			dslb.descriptorType = ConvertShaderParameterTypeToVulkan(parameter.GetType());

			if (!layout_bindings.Add(dslb))
			{
				return ut::MakeError(ut::error::out_of_memory);
			}
		}
	}

	// initialize descriptor set layout
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSetLayoutCreateInfo dsl_info;
	dsl_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	dsl_info.pNext = nullptr;
	dsl_info.flags = 0;
	dsl_info.bindingCount = static_cast<uint32_t>(layout_bindings.GetNum());
	dsl_info.pBindings = layout_bindings.GetAddress();
	VkResult res = vkCreateDescriptorSetLayout(device.GetVkHandle(), &dsl_info, nullptr, &descriptor_set_layout);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateDescriptorSetLayout"));
	}

	// create pipeline layout
	VkPipelineLayout pipeline_layout;
	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.pushConstantRangeCount = 0;
	pipeline_layout_info.pPushConstantRanges = nullptr;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
	res = vkCreatePipelineLayout(device.GetVkHandle(), &pipeline_layout_info, nullptr, &pipeline_layout);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreatePipelineLayout"));
	}

	// initialize pipeline create info
	VkGraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	pipeline_info.basePipelineIndex = 0;
	pipeline_info.flags = 0;
	pipeline_info.stageCount = shader_stage_count;
	pipeline_info.pStages = shader_stages.GetAddress();
	pipeline_info.pVertexInputState = &vertex_input;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterization_state;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &ds_state;
	pipeline_info.pColorBlendState = &blend_state;
	pipeline_info.pTessellationState = info.tessellation_enable ? &tessellation_state : nullptr;
	pipeline_info.pDynamicState = nullptr;
	pipeline_info.renderPass = render_pass.GetVkHandle();
	pipeline_info.subpass = 0;

	// create vulkan pipeline
	VkPipeline pipeline;
	res = vkCreateGraphicsPipelines(device.GetVkHandle(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateGraphicsPipelines"));
	}

	// create descriptor pool

	// success
	PlatformPipelineState platform_pipeline_state(device.GetVkHandle(), pipeline, pipeline_layout, descriptor_set_layout);
	return PipelineState(ut::Move(platform_pipeline_state), ut::Move(info));
}

// Resets given command buffer. This command buffer must be created without
// CmdBufferInfo::usage_dynamic flag (use WaitCmdBuffer() instead).
//    @param cmd_buffer - reference to the buffer to be reset.
void Device::ResetCmdBuffer(CmdBuffer& cmd_buffer)
{
	if (cmd_buffer.info.usage & CmdBuffer::usage_dynamic)
	{
		throw ut::Error(ut::error::invalid_arg, "Render: provided command buffer can't be reset.");
	}
	VkResult res = vkResetCommandBuffer(cmd_buffer.GetVkHandle(), 0);
	UT_ASSERT(res == VK_SUCCESS);
}

// Records commands by calling a provided function.
//    @param cmd_buffer - reference to the buffer to record commands to.
//    @param function - function containing commands to record.
//    @param render_pass - optional reference to the current renderpass,
//                         this parameter applies only for secondary buffers
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
//    @param framebuffer - optional reference to the current framebuffer,
//                         this parameter applies only for secondary buffers
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
void Device::Record(CmdBuffer& cmd_buffer,
                    ut::Function<void(Context&)> function,
                    ut::Optional<RenderPass&> render_pass,
                    ut::Optional<Framebuffer&> framebuffer)
{
	VkCommandBufferUsageFlags flags = 0;

	if (cmd_buffer.info.usage & CmdBuffer::usage_dynamic)
	{
		flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}

	if (cmd_buffer.info.usage & CmdBuffer::usage_inside_render_pass)
	{
		flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	}

	VkCommandBufferBeginInfo cmd_buffer_info = {};
	cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buffer_info.pNext = nullptr;
	cmd_buffer_info.flags = flags;

	// inheritance must be set for secondary buffers
	VkCommandBufferInheritanceInfo cmd_buf_inheritance_info = {};
	if (cmd_buffer.info.usage & CmdBuffer::level_secondary)
	{
		cmd_buf_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, cmd_buf_inheritance_info.pNext = NULL;
		cmd_buf_inheritance_info.subpass = 0;
		cmd_buf_inheritance_info.occlusionQueryEnable = VK_FALSE;
		cmd_buf_inheritance_info.queryFlags = 0;
		cmd_buf_inheritance_info.pipelineStatistics = 0;

		// render pass and framebuffer must be set for secondary buffers
		// with CmdBufferInfo::usage_inside_render_pass flag enabled
		if (cmd_buffer.info.usage & CmdBuffer::usage_inside_render_pass)
		{
			if (render_pass && framebuffer)
			{
				cmd_buf_inheritance_info.renderPass = render_pass->GetVkHandle();
				cmd_buf_inheritance_info.framebuffer = framebuffer->GetVkHandle();
			}
			else
			{
				throw ut::Error(ut::error::invalid_arg, "Vulkan: no renderpass and(or) framebuffer for secondary buffer.");
			}
		}
		else
		{
			cmd_buf_inheritance_info.renderPass = VK_NULL_HANDLE;
			cmd_buf_inheritance_info.framebuffer = VK_NULL_HANDLE;
		}
	}
	else
	{
		cmd_buffer_info.pInheritanceInfo = nullptr;
	}

	// start recording commands
	VkResult res = vkBeginCommandBuffer(cmd_buffer.GetVkHandle(), &cmd_buffer_info);
	UT_ASSERT(res == VK_SUCCESS);

	// call function
	Context context(PlatformContext(device.GetVkHandle(), cmd_buffer));
	function(context);

	// stop recording
	res = vkEndCommandBuffer(cmd_buffer.GetVkHandle());
	UT_ASSERT(res == VK_SUCCESS);
}

// Waits for all commands from the provided buffer to be executed.
//    @param cmd_buffer - reference to the command buffer.
void Device::WaitCmdBuffer(CmdBuffer& cmd_buffer)
{
	VkResult res = VK_SUCCESS;

	// exit if buffer wasn't submitted
	if (!cmd_buffer.pending)
	{
		return;
	}

	// wait for fence
	VkFence fences[] = { cmd_buffer.fence.GetVkHandle() };
	do {
		res = vkWaitForFences(device.GetVkHandle(), 1, fences, VK_TRUE, skFenceInfiniteTimeout);
	} while (res == VK_TIMEOUT);
	UT_ASSERT(res == VK_SUCCESS);

	// reset fences
	vkResetFences(device.GetVkHandle(), 1, fences);

	// reset pool
	if (cmd_buffer.info.usage & CmdBuffer::usage_dynamic)
	{
		res = vkResetCommandPool(device.GetVkHandle(), cmd_buffer.pool.GetVkHandle(), 0);
		UT_ASSERT(res == VK_SUCCESS);
	}

	// reset bound pipeline handle
	cmd_buffer.bound_pipeline = ut::Optional<PlatformPipelineState&>();

	// reset descriptor pools
	cmd_buffer.descriptor_mgr.Reset();

	// reset command buffer pending status
	cmd_buffer.pending = false;
}

// Submits a command buffer to a queue. Also it's possible to enqueue presentation
// for displays used in the provided command buffer. Presentation is supported
// only for command buffers created with CmdBufferInfo::usage_dynamic flag.
//    @param cmd_buffer - reference to the command buffer to enqueue.
//    @param present_queue - array of references to displays waiting for their
//                           buffer to be presented to user.
void Device::Submit(CmdBuffer& cmd_buffer,
                    ut::Array< ut::Ref<Display> > present_queue)
{
	// check if cmd buffer already submitted commands
	if (cmd_buffer.pending)
	{
		throw ut::Error(ut::error::fail, "Render: Command buffer is already in use.");
	}

	// check if cmd buffer is a primary buffer
	if (cmd_buffer.info.level != CmdBuffer::level_primary)
	{
		throw ut::Error(ut::error::invalid_arg, "Render: secondary buffers can't be submit.");
	}

	// if present queue isn't empty - check if cmd buffer is a dynamic buffer
	const ut::uint32 display_count = static_cast<ut::uint32>(present_queue.GetNum());
	if (display_count != 0 && !(cmd_buffer.info.usage & CmdBuffer::usage_dynamic))
	{
		throw ut::Error(ut::error::invalid_arg, "Render: only dynamic command buffers support present.");
	}

	// initialize synchronization primitives
	VkSemaphore wait_semaphores[skMaxVkSwapchainPresent];
	VkPipelineStageFlags wait_stages[skMaxVkSwapchainPresent];
	VkSemaphore signal_semaphores[skMaxVkSwapchainPresent];
	for (ut::uint32 i = 0; i < display_count; i++)
	{
		Display& display = present_queue[i].Get();

		ut::uint32 swap_id = display.swap_count;
		wait_semaphores[i] = display.availability_semaphores[swap_id].GetVkHandle();
		wait_stages[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		const ut::uint32 current_buffer_id = display.current_buffer_id;
		signal_semaphores[i] = display.present_ready_semaphores[current_buffer_id].GetVkHandle();

		if (display.pending_buffers[current_buffer_id])
		{
			throw ut::Error(ut::error::invalid_arg, "Render: Display command buffer isn't finished for the previous frame.");
		}
	}

	// initialize submit info
	const VkCommandBuffer cmd_bufs[] = { cmd_buffer.GetVkHandle() };
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = nullptr;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = display_count;
	submit_info[0].pWaitSemaphores = wait_semaphores;
	submit_info[0].pWaitDstStageMask = wait_stages;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = display_count;
	submit_info[0].pSignalSemaphores = signal_semaphores;

	// get command buffer fence
	VkFence fence = cmd_buffer.fence.GetVkHandle();

	// queue the command buffer for execution
	VkResult res = vkQueueSubmit(main_queue.GetVkHandle(), 1, submit_info, fence);
	UT_ASSERT(res == VK_SUCCESS);

	// mark command buffer as one pending execution, this flag will be
	// reset to 'false' only after a user calls WaitCmdBuffer()
	cmd_buffer.pending = true;

	// form swapchain array and the array of wait semaphores
	VkSwapchainKHR swapchains[skMaxVkSwapchainPresent];
	uint32_t swapchain_buffer_indices[skMaxVkSwapchainPresent];
	for (ut::uint32 i = 0; i < display_count; i++)
	{
		const uint32_t id = present_queue[i]->current_buffer_id;
		swapchain_buffer_indices[i] = id;
		swapchains[i] = present_queue[i]->swap_chain.GetVkHandle();
	}

	// enqueue presentation of this frame
	if (display_count != 0)
	{
		// present displays
		VkPresentInfoKHR present;
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.pNext = nullptr;
		present.swapchainCount = display_count;
		present.pSwapchains = swapchains;
		present.pImageIndices = swapchain_buffer_indices;
		present.waitSemaphoreCount = display_count;
		present.pWaitSemaphores = signal_semaphores;
		present.pResults = nullptr;

		// present
		res = vkQueuePresentKHR(main_queue.GetVkHandle(), &present);
		if (res == VK_SUCCESS)
		{
			// add information about the command buffer associated with a
			// particular display buffer
			for (ut::uint32 i = 0; i < display_count; i++)
			{
				const ut::uint32 current_buffer_id = present_queue[i]->current_buffer_id;
				present_queue[i]->pending_buffers[current_buffer_id] = cmd_buffer;
			}
		}
		else if (res != VK_ERROR_OUT_OF_DATE_KHR && res != VK_SUBOPTIMAL_KHR)
		{
			UT_ASSERT(0);
		}
	}
}

// Acquires next buffer in a swapchain to be filled.
//    @param display - reference to the display.
void Device::AcquireNextDisplayBuffer(Display& display)
{
	ut::uint32 next_buffer_id = display.current_buffer_id + 1;
	if (next_buffer_id >= display.targets.GetNum())
	{
		next_buffer_id = 0;
	}

	// finish command buffer working with the next display buffer
	ut::Optional<CmdBuffer&>& pending_cmd_buffer = display.pending_buffers[next_buffer_id];
	if (pending_cmd_buffer)
	{
		WaitCmdBuffer(pending_cmd_buffer.Get());
		pending_cmd_buffer = ut::Optional<CmdBuffer&>();
	}

	// acquire next buffer
	display.current_buffer_id = display.AcquireNextBuffer();
}

// Call this function to wait on the host for the completion of all
// queue operations for all queues on this device.
void Device::WaitIdle()
{
	vkDeviceWaitIdle(device.GetVkHandle());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//