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

	// helper struct to store actual resource info
	union WriteDescriptorSetInfo
	{
		VkDescriptorImageInfo image;
		VkDescriptorBufferInfo buffer;
		VkBufferView text_buffer;
	};

	// initialize descriptor set write info
	const size_t descriptor_count = set.GetDescriptorCount();
	ut::Array<VkWriteDescriptorSet> write_actions(descriptor_count);
	ut::Array<WriteDescriptorSetInfo> write_info(descriptor_count);
	uint32_t write_count = 0;
	for (size_t i = 0; i < descriptor_count; i++)
	{
		Descriptor& descriptor = set.GetDescriptor(i);

		ut::Optional<Descriptor::Binding> binding = descriptor.GetBinding();
		if (!binding)
		{
			continue;
		}

		if (!binding->slot)
		{
			ut::log.Lock() << "Warning! Descriptor binding isn't set: \"" <<
			               descriptor.name << "\"" << ut::cret;
			continue;
		}

		VkWriteDescriptorSet descriptor_write;
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.pNext = nullptr;
		descriptor_write.dstSet = vk_desc_set_handle;
		descriptor_write.dstBinding = binding->id;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorCount = 1;
		descriptor_write.descriptorType = ConvertShaderParameterTypeToVulkan(binding->type);
		descriptor_write.pImageInfo = nullptr;
		descriptor_write.pBufferInfo = nullptr;
		descriptor_write.pTexelBufferView = nullptr;

		if (binding->type == Shader::Parameter::uniform_buffer)
		{
			Buffer* uniform_buffer = binding->slot->uniform_buffer;
			write_info[i].buffer.buffer = uniform_buffer->GetVkHandle();
			write_info[i].buffer.offset = 0;
			write_info[i].buffer.range = uniform_buffer->GetInfo().size;
			descriptor_write.pBufferInfo = &write_info[i].buffer;
		}
		else
		{
			ut::log.Lock() << "Error! Vulkan descriptor write action is not implemented." << ut::cret;
			throw ut::Error(ut::error::not_implemented);
		}

		write_actions[i] = descriptor_write;
		write_count++;
	}

	// update descriptor set
	if (write_count > 0)
	{
		vkUpdateDescriptorSets(device, write_count, write_actions.GetAddress(), 0, nullptr);
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

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//