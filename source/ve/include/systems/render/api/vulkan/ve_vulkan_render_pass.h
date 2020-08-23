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
// Vulkan renderpass.
class PlatformRenderPass : public VkRc<vk::render_pass>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformRenderPass(VkDevice device_handle, VkRenderPass render_pass_handle);

	// Move constructor.
	PlatformRenderPass(PlatformRenderPass&&) noexcept;

	// Move operator.
	PlatformRenderPass& operator =(PlatformRenderPass&&) noexcept;

	// Copying is prohibited.
	PlatformRenderPass(const PlatformRenderPass&) = delete;
	PlatformRenderPass& operator =(const PlatformRenderPass&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//