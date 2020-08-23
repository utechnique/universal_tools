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
// Vulkan pipeline.
class PlatformPipelineState
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformPipelineState(VkDevice device_handle,
	                      VkPipeline pipeline_handle,
	                      VkPipelineLayout layout_handle,
	                      VkDescriptorSetLayout dsl_handle);

	// Move constructor.
	PlatformPipelineState(PlatformPipelineState&&) noexcept;

	// Move operator.
	PlatformPipelineState& operator =(PlatformPipelineState&&) noexcept;

	// Copying is prohibited.
	PlatformPipelineState(const PlatformPipelineState&) = delete;
	PlatformPipelineState& operator =(const PlatformPipelineState&) = delete;

private:
	VkRc<vk::pipeline> pipeline;
	VkRc<vk::pipeline_layout> layout;
	VkRc<vk::descriptor_set_layout> dsl;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//