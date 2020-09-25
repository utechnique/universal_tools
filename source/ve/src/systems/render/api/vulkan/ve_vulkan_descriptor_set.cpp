//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_descriptor_set.h"
#include "systems/render/api/ve_render_descriptor.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts shader parameter type to the Vulkan descriptor type.
VkDescriptorType ConvertShaderParameterTypeToVulkan(ut::uint32 type)
{
	switch (type)
	{
	case Shader::Parameter::uniform_buffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case Shader::Parameter::image:          return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case Shader::Parameter::sampler:        return VK_DESCRIPTOR_TYPE_SAMPLER;
	case Shader::Parameter::storage_buffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

//----------------------------------------------------------------------------//
// Constructor, allocates memory for the pool.
DescriptorPool::DescriptorPool(VkDevice device_handle) : device(device_handle)
{
	VkDescriptorPoolSize rc_sizes[4];
	rc_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	rc_sizes[0].descriptorCount = skDefaultPoolSize;
	rc_sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	rc_sizes[1].descriptorCount = skDefaultPoolSize;
	rc_sizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	rc_sizes[2].descriptorCount = skDefaultPoolSize;
	rc_sizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	rc_sizes[3].descriptorCount = skDefaultPoolSize;

	VkDescriptorPoolCreateInfo pool_info;
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.pNext = nullptr;
	pool_info.flags = 0;
	pool_info.pPoolSizes = rc_sizes;
	pool_info.poolSizeCount = 4;
	pool_info.maxSets = skDefaultPoolSize;

	VkDescriptorPool pool_handle;
	VkResult res = vkCreateDescriptorPool(device, &pool_info, nullptr, &pool_handle);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateDescriptorPool");
	}

	pool = VkRc<vk::descriptor_pool>(pool_handle, device);
}

// Move constructor.
DescriptorPool::DescriptorPool(DescriptorPool&&) noexcept = default;

// Move operator.
DescriptorPool& DescriptorPool::operator =(DescriptorPool&&) noexcept = default;

// Allocates a descriptor set, may return nothing if
// there is no space left in the pool.
ut::Optional<VkDescriptorSet> DescriptorPool::AllocateSet(DescriptorSet& set,
                                                          VkDescriptorSetLayout layout_handle)
{
	// check if there is enough space for a new set
	if (set_counter >= skDefaultPoolSize)
	{
		return ut::Optional<VkDescriptorSet>();
	}

	// check if there is enough space for all descriptors
	DescriptorSet::ResourceCount count = set.GetResourceCount();
	if (uniform_buffer_counter + count.uniform_buffers > skDefaultPoolSize ||
		image_counter          + count.images          > skDefaultPoolSize ||
		sampler_counter        + count.samplers        > skDefaultPoolSize ||
		storage_buffer_counter + count.storage_buffers > skDefaultPoolSize)
	{
		return ut::Optional<VkDescriptorSet>();
	}

	// initialize allocate info
	VkDescriptorSetAllocateInfo alloc_info;
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.pNext = nullptr;
	alloc_info.descriptorPool = pool.GetVkHandle();
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &layout_handle;

	// allocate set
	VkDescriptorSet set_handle;
	VkResult res = vkAllocateDescriptorSets(device, &alloc_info, &set_handle);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkAllocateDescriptorSets");
	}

	// update counters
	set_counter++;
	uniform_buffer_counter += count.uniform_buffers;
	image_counter += count.images;
	sampler_counter += count.samplers;
	storage_buffer_counter += count.storage_buffers;

	// success
	return set_handle;
}

// Deallocates descriptor sets.
void DescriptorPool::Reset()
{
	VkResult res = vkResetDescriptorPool(device, pool.GetVkHandle(), 0);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkResetDescriptorPool");
	}

	uniform_buffer_counter = 0;
	image_counter = 0;
	sampler_counter = 0;
	storage_buffer_counter = 0;
	set_counter = 0;
}

//----------------------------------------------------------------------------//
// Constructor.
DescriptorManager::DescriptorManager(VkDevice device_handle,
                                     VkCommandBuffer cmd_buffer_handle) : device(device_handle)
                                                                        , cmd_buffer(cmd_buffer_handle)
{}

// Move constructor.
DescriptorManager::DescriptorManager(DescriptorManager&&) noexcept = default;

// Move operator.
DescriptorManager& DescriptorManager::operator =(DescriptorManager&&) noexcept = default;

// Allocates a descriptor set then binds it to the pipeline.
void DescriptorManager::AllocateAndBindDescriptorSet(DescriptorSet& set,
                                                     VkDescriptorSetLayout desc_set_layout_handle,
                                                     VkPipelineLayout pipeline_layout_handle)
{
	// try to allocate set using one of the existing pools
	ut::Optional<VkDescriptorSet> vk_desc_set;
	const size_t pool_count = pools.GetNum();
	for (size_t i = 0; i < pool_count; i++)
	{
		vk_desc_set = pools[i].AllocateSet(set, desc_set_layout_handle);
		if (vk_desc_set)
		{
			break;
		}
	}

	// if all pools are all filled to the max, allocate a new pool
	if (!vk_desc_set)
	{
		if (!pools.Add(DescriptorPool(device)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}

		// try again with a new pool
		vk_desc_set = pools.GetLast().AllocateSet(set, desc_set_layout_handle);
		if (!vk_desc_set)
		{
			throw ut::Error(ut::error::fail, "Vulkan: unable to allocate a descriptor set.");
		}
	}

	// get vulkan handle to the descriptor
	VkDescriptorSet vk_desc_set_handle = vk_desc_set.Get();

	// initialize descriptor set write info
	const size_t descriptor_count = set.GetDescriptorCount();
	const size_t slot_count = set.GetSlotCount();

	// update cache
	UpdateCacheSize(descriptor_count, slot_count);
	ut::Optional<ut::Error> cache_error = StoreWriteInfoInCache(set, vk_desc_set_handle);

	// update descriptor set
	if (!cache_error)
	{
		vkUpdateDescriptorSets(device,
		                       static_cast<uint32_t>(descriptor_count),
		                       write_cache.GetAddress(),
		                       0,
		                       nullptr);
	}
	
	// bind to the pipeline
	vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_handle, 0, 1, &vk_desc_set_handle, 0, nullptr);
}

// Resets all pools.
void DescriptorManager::Reset()
{
	const size_t pool_count = pools.GetNum();
	for (size_t i = 0; i < pool_count; i++)
	{
		pools[i].Reset();
	}
}

// Updates size of internal caches to be able to update a descriptor set.
//    @param descriptor_count - number of descriptors in a set.
//    @paramseparate_element_count - number of separate parameters in a set.
void DescriptorManager::UpdateCacheSize(size_t descriptor_count,
                                        size_t separate_element_count)
{
	if (write_cache.GetNum() < descriptor_count)
	{
		if (!write_cache.Resize(descriptor_count))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
	if (buffer_cache.GetNum() < separate_element_count)
	{
		if (!buffer_cache.Resize(separate_element_count))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
	if (image_cache.GetNum() < separate_element_count)
	{
		if (!image_cache.Resize(separate_element_count))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
}

// Updates cache with write info data.
ut::Optional<ut::Error> DescriptorManager::StoreWriteInfoInCache(const DescriptorSet& set,
                                                                 VkDescriptorSet handle)
{
	// number of separate parameters written to the cache for this set
	size_t elements_written = 0;

	// iterate all descriptors
	const size_t descriptor_count = set.GetDescriptorCount();
	for (size_t i = 0; i < descriptor_count; i++)
	{
		const Descriptor& descriptor = set.GetDescriptor(i);

		// initialize VkWriteDescriptorSet object
		VkWriteDescriptorSet write_set;
		ut::Optional<size_t> array_elements = InitializeWriteDescriptorSet(write_set,
		                                                                   descriptor,
		                                                                   handle,
		                                                                   elements_written);
		if (!array_elements)
		{
			return ut::Error(ut::error::fail);
		}

		// update element counter
		elements_written += array_elements.Get();

		// write current descriptor to the cache
		write_cache[i] = write_set;
	}

	// success
	return ut::Optional<ut::Error>();
}

// Initializes VkWriteDescriptorSet object.
//    @param wds - reference to the VkWriteDescriptorSet object
//                 to be initialized.
//    @param descriptor - const reference to the descriptor to
//                        initialize VkWriteDescriptorSet with.
//    @param handle - vulkan descriptor handle.
//    @param cache_offset - number of separate elements written
//                          before this descriptor.
//    @return - number of elements written or nothing if failed.
ut::Optional<size_t> DescriptorManager::InitializeWriteDescriptorSet(VkWriteDescriptorSet& wds,
                                                                     const Descriptor& descriptor,
                                                                     VkDescriptorSet handle,
                                                                     size_t cache_offset)
{
	// check if appropriate binding exists
	const ut::Optional<Descriptor::Binding>& binding = descriptor.GetBinding();
	if (!binding)
	{
		ut::log.Lock() << "Vulkan: Error! Descriptor binding isn't set: \"" <<
		                  descriptor.name << ut::cret;
		return ut::Optional<size_t>();
	}

	const size_t array_elements = binding->slots.GetNum();

	// initialize VkWriteDescriptorSet object
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.pNext = nullptr;
	wds.dstSet = handle;
	wds.dstBinding = binding->id;
	wds.dstArrayElement = 0;
	wds.descriptorCount = static_cast<uint32_t>(array_elements);
	wds.descriptorType = ConvertShaderParameterTypeToVulkan(binding->type);
	wds.pImageInfo = nullptr;
	wds.pBufferInfo = nullptr;
	wds.pTexelBufferView = nullptr;

	// remember current position in cache
	VkDescriptorBufferInfo* buffer_cache_entry = &buffer_cache[cache_offset];
	VkDescriptorImageInfo* image_cache_entry = &image_cache[cache_offset];

	// iterate all array elements
	for (size_t j = 0; j < array_elements; j++)
	{
		const ut::Optional<Descriptor::Slot>& slot = binding->slots[j];

		// check if array element is bound
		if (!slot)
		{
			ut::log.Lock() << "Vulkan: Error! Descriptor binding slot isn't set: \"" <<
			                  descriptor.name << "\" " << j << ut::cret;
			return ut::Optional<size_t>();
		}

		// calculate position of the current element in cache
		VkDescriptorBufferInfo& buffer_element_info = buffer_cache_entry[j];
		VkDescriptorImageInfo& img_element_info = image_cache_entry[j];

		// initialize buffer or image information according to the
		// appropriate shader parameter type
		if (binding->type == Shader::Parameter::uniform_buffer)
		{
			Buffer* uniform_buffer = slot->uniform_buffer;
			buffer_element_info.buffer = uniform_buffer->GetVkHandle();
			buffer_element_info.offset = 0;
			buffer_element_info.range = uniform_buffer->GetInfo().size;
			wds.pBufferInfo = buffer_cache_entry;
		}
		else if (binding->type == Shader::Parameter::image)
		{
			if (slot->cube_face)
			{
				img_element_info.imageView = slot->image->GetCubeFaceView(slot->cube_face.Get());
			}
			else
			{
				img_element_info.imageView = slot->image->GetView();
			}

			img_element_info.imageLayout = slot->image->GetLayout();
			wds.pImageInfo = image_cache_entry;
		}
		else if (binding->type == Shader::Parameter::sampler)
		{
			img_element_info.sampler = slot->sampler->GetVkHandle();
			img_element_info.imageView = VK_NULL_HANDLE;
			wds.pImageInfo = image_cache_entry;
		}
		else
		{
			ut::log.Lock() << "Error! Vulkan descriptor write action is not implemented." << ut::cret;
			throw ut::Error(ut::error::not_implemented);
		}
	}

	return array_elements;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//