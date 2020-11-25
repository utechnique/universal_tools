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
// Vulkan framebuffer.
class PlatformFramebuffer : public VkRc<vk::framebuffer>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformFramebuffer(VkDevice device_handle,
	                    VkFramebuffer framebuffer_handle);

	// Move constructor.
	PlatformFramebuffer(PlatformFramebuffer&&) noexcept;

	// Move operator.
	PlatformFramebuffer& operator =(PlatformFramebuffer&&) noexcept;

	// Copying is prohibited.
	PlatformFramebuffer(const PlatformFramebuffer&) = delete;
	PlatformFramebuffer& operator =(const PlatformFramebuffer&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//