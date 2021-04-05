//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_target.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformRenderTarget::PlatformRenderTarget(ut::Array<SliceRTV> in_rtv_slices) : slice_target_views(ut::Move(in_rtv_slices))
{}

// Move constructor.
PlatformRenderTarget::PlatformRenderTarget(PlatformRenderTarget&&) noexcept = default;

// Move operator.
PlatformRenderTarget& PlatformRenderTarget::operator =(PlatformRenderTarget&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//