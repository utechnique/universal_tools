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
PlatformRenderTarget::PlatformRenderTarget(ut::Array<SliceView> in_slice_views) : slice_views(ut::Move(in_slice_views))
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