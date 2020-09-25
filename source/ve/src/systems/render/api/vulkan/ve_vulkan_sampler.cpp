//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_sampler.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformSampler::PlatformSampler(VkDevice device_handle,
                                 VkSampler sampler_handle) : VkRc<vk::sampler>(sampler_handle, device_handle)
{}

// Move constructor.
PlatformSampler::PlatformSampler(PlatformSampler&&) noexcept = default;

// Move operator.
PlatformSampler& PlatformSampler::operator =(PlatformSampler&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//