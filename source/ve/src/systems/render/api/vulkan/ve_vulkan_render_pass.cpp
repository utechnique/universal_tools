//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_pass.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformRenderPass::PlatformRenderPass(VkDevice device_handle,
                                       VkRenderPass render_pass_handle) : render_pass(render_pass_handle, device_handle)
{}

// Move constructor.
PlatformRenderPass::PlatformRenderPass(PlatformRenderPass&&) noexcept = default;

// Move operator.
PlatformRenderPass& PlatformRenderPass::operator =(PlatformRenderPass&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//