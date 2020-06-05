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
// Vulkan render target.
class PlatformRenderTarget
{
	friend class Device;
	friend class Context;
	friend class PlatformDisplay;
public:
	PlatformRenderTarget(VkDevice device_handle,
	                     VkImageView image_view_handle);

	// Move constructor.
	PlatformRenderTarget(PlatformRenderTarget&&) noexcept;

	// Move operator.
	PlatformRenderTarget& operator =(PlatformRenderTarget&&) noexcept;

	// Copying is prohibited.
	PlatformRenderTarget(const PlatformRenderTarget&) = delete;
	PlatformRenderTarget& operator =(const PlatformRenderTarget&) = delete;

private:
	VkRc<vk::image_view> image_view;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//