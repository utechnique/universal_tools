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
PlatformRenderTarget::PlatformRenderTarget(ID3D11RenderTargetView* rtv_ptr,
                                           ID3D11DepthStencilView* dsv_ptr) : rtv(rtv_ptr)
                                                                            , dsv(dsv_ptr)
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