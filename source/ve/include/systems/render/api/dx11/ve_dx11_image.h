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
// DirectX 11 image.
class PlatformImage
{
	friend class Device;
	friend class Context;
	friend class PlatformContext;
public:
	// Constructor, accepts 1d texture.
	explicit PlatformImage(ID3D11Texture1D* t1d_ptr,
	                       ID3D11ShaderResourceView* srv_ptr);

	// Constructor, accepts 2d texture.
	explicit PlatformImage(ID3D11Texture2D* t2d_ptr,
	                       ID3D11ShaderResourceView* srv_ptr,
	                       ID3D11ShaderResourceView** cube_faces_ptr = nullptr);

	// Constructor, accepts 3d texture.
	explicit PlatformImage(ID3D11Texture3D* t3d_ptr,
	                       ID3D11ShaderResourceView* srv_ptr);

	// Move constructor.
	PlatformImage(PlatformImage&&) noexcept;

	// Move operator.
	PlatformImage& operator =(PlatformImage&&) noexcept;

	// Copying is prohibited.
	PlatformImage(const PlatformImage&) = delete;
	PlatformImage& operator =(const PlatformImage&) = delete;

private:
	ut::ComPtr<ID3D11ShaderResourceView> srv;
	ut::ComPtr<ID3D11Texture1D>			 tex1d;
	ut::ComPtr<ID3D11Texture2D>			 tex2d;
	ut::ComPtr<ID3D11Texture3D>			 tex3d;
	ut::ComPtr<ID3D11Texture2D>			 stage2d;
	ut::ComPtr<ID3D11ShaderResourceView> cube_faces[6];
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//