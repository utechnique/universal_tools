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
// DirectX 11 texture.
class PlatformTexture
{
	friend class Device;
	friend class Context;
public:
	// Constructor, accepts 1d texture.
	explicit PlatformTexture(ID3D11Texture1D* t1d_ptr,
	                         ID3D11ShaderResourceView* srv_ptr);

	// Constructor, accepts 2d texture.
	explicit PlatformTexture(ID3D11Texture2D* t2d_ptr,
	                         ID3D11ShaderResourceView* srv_ptr);

	// Constructor, accepts 3d texture.
	explicit PlatformTexture(ID3D11Texture3D* t3d_ptr,
	                         ID3D11ShaderResourceView* srv_ptr);

	// Move constructor.
	PlatformTexture(PlatformTexture&&) noexcept;

	// Move operator.
	PlatformTexture& operator =(PlatformTexture&&) noexcept;

	// Copying is prohibited.
	PlatformTexture(const PlatformTexture&) = delete;
	PlatformTexture& operator =(const PlatformTexture&) = delete;

private:
	ut::ComPtr<ID3D11ShaderResourceView> srv;
	ut::ComPtr<ID3D11Texture1D>			 tex1d;
	ut::ComPtr<ID3D11Texture2D>			 tex2d;
	ut::ComPtr<ID3D11Texture3D>			 tex3d;
	ut::ComPtr<ID3D11Texture2D>			 stage2d;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//