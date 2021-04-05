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
	// Manages both render target view and depth-stencil view.
	struct CombinedRTV
	{
		ut::ComPtr<ID3D11RenderTargetView> rtv;
		ut::ComPtr<ID3D11DepthStencilView> dsv;
	};

	// A set of render target views for all mips in a texture slice. 
	struct SliceRTV
	{
		ut::Array<CombinedRTV> mips;
	};

	// Constructor.
	PlatformRenderTarget(ut::Array<SliceRTV> in_rtv_slices);

	// Move constructor.
	PlatformRenderTarget(PlatformRenderTarget&&) noexcept;

	// Move operator.
	PlatformRenderTarget& operator =(PlatformRenderTarget&&) noexcept;

	// Copying is prohibited.
	PlatformRenderTarget(const PlatformRenderTarget&) = delete;
	PlatformRenderTarget& operator =(const PlatformRenderTarget&) = delete;

private:
	ut::Array<SliceRTV> slice_target_views;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//