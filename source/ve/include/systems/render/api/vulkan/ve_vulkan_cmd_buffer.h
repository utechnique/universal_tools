//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
#include "systems/render/api/vulkan/ve_vulkan_descriptor_set.h"
#include "systems/render/api/vulkan/ve_vulkan_pipeline_state.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan command buffer.
class PlatformCmdBuffer : public VkRc<vk::cmd_buffer>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformCmdBuffer(VkDevice device_handle,
	                  VkCommandPool cmd_pool_handle,
	                  VkCommandBuffer cmd_buffer_handle,
	                  VkFence fence_handle);

	// Destructor.
	~PlatformCmdBuffer();

	// Move constructor.
	PlatformCmdBuffer(PlatformCmdBuffer&&) noexcept;

	// Move operator.
	PlatformCmdBuffer& operator =(PlatformCmdBuffer&&) noexcept;

	// Copying is prohibited.
	PlatformCmdBuffer(const PlatformCmdBuffer&) = delete;
	PlatformCmdBuffer& operator =(const PlatformCmdBuffer&) = delete;

private:
	VkRc<vk::cmd_pool> pool;
	VkRc<vk::fence> fence;
	DescriptorManager descriptor_mgr;
	ut::Optional<PlatformPipelineState&> bound_pipeline;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//