//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_target.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformRenderTarget::PlatformRenderTarget(VkDevice device_handle,
                                           VkImageView image_view_handle) : image_view(image_view_handle, device_handle)
{}

// Move constructor.
PlatformRenderTarget::PlatformRenderTarget(PlatformRenderTarget&&) noexcept = default;

// Move operator.
PlatformRenderTarget& PlatformRenderTarget::operator =(PlatformRenderTarget&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//