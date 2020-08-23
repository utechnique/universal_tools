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
// Forward declarations.
class DescriptorSet;

//----------------------------------------------------------------------------//
// Converts shader parameter type to the Vulkan descriptor type.
VkDescriptorType ConvertShaderParameterTypeToVulkan(ut::uint32 type);

//----------------------------------------------------------------------------//
// ve::render::DescriptorPool is a helper class-wrapper around VkDescriptorPool.
class DescriptorPool
{
public:
	// Constructor, allocates memory for the pool.
	DescriptorPool(VkDevice device_handle);

	// Move constructor.
	DescriptorPool(DescriptorPool&&) noexcept;

	// Move operator.
	DescriptorPool& operator =(DescriptorPool&&) noexcept;

	// Copying is prohibited.
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator =(const DescriptorPool&) = delete;

	// Allocates a descriptor set, may return nothing if
	// there is no space left in the pool.
	ut::Optional<VkDescriptorSet> AllocateSet(DescriptorSet& set,
	                                          VkDescriptorSetLayout layout_handle);

	// Deallocates descriptor sets.
	void Reset();

	// maximum number of total resource binding of each type
	// and maximum number of sets that can be allocated
	static const ut::uint32 skDefaultPoolSize = 1024;

	// counters
	ut::uint32 uniform_buffer_counter = 0;
	ut::uint32 image_counter = 0;
	ut::uint32 sampler_counter = 0;
	ut::uint32 storage_buffer_counter = 0;
	ut::uint32 set_counter = 0;

private:
	VkRc<vk::descriptor_pool> pool;
	VkDevice device;
};

//----------------------------------------------------------------------------//
// ve::render::DescriptorManager is a class encapsulating operations with
// descriptors. It can allocate and bind descriptors to the pipeline.
class DescriptorManager
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	DescriptorManager(VkDevice device_handle, VkCommandBuffer cmd_buffer_handle);

	// Move constructor.
	DescriptorManager(DescriptorManager&&) noexcept;

	// Move operator.
	DescriptorManager& operator =(DescriptorManager&&) noexcept;

	// Copying is prohibited.
	DescriptorManager(const DescriptorManager&) = delete;
	DescriptorManager& operator =(const DescriptorManager&) = delete;

	// Allocates a descriptor set then binds it to the pipeline.
	void AllocateAndBindDescriptorSet(DescriptorSet& set,
	                                  VkDescriptorSetLayout desc_set_layout_handle,
	                                  VkPipelineLayout pipeline_layout_handle);

	// Resets all pools.
	void Reset();

private:
	ut::Array<DescriptorPool> pools;
	VkDevice device;
	VkCommandBuffer cmd_buffer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//