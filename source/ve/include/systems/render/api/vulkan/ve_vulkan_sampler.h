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
// Vulkan sampler.
class PlatformSampler : public VkRc<vk::sampler>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformSampler(VkDevice device_handle,
	                VkSampler sampler_handle);

	// Move constructor.
	PlatformSampler(PlatformSampler&&) noexcept;

	// Move operator.
	PlatformSampler& operator =(PlatformSampler&&) noexcept;

	// Copying is prohibited.
	PlatformSampler(const PlatformSampler&) = delete;
	PlatformSampler& operator =(const PlatformSampler&) = delete;

private:

};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//