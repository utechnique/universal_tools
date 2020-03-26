//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "ut.h"
#include <d3d11.h>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// DirectX11 render target.
class PlatformRenderTarget
{
	friend class Device;
	friend class Context;
public:
	PlatformRenderTarget(ID3D11RenderTargetView* rtv_ptr,
	                     ID3D11DepthStencilView* dsv_ptr);

	// Move constructor.
	PlatformRenderTarget(PlatformRenderTarget&&) noexcept;

	// Move operator.
	PlatformRenderTarget& operator =(PlatformRenderTarget&&) noexcept;

	// Copying is prohibited.
	PlatformRenderTarget(const PlatformRenderTarget&) = delete;
	PlatformRenderTarget& operator =(const PlatformRenderTarget&) = delete;

private:
	ut::ComPtr<ID3D11RenderTargetView> rtv;
	ut::ComPtr<ID3D11DepthStencilView> dsv;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//