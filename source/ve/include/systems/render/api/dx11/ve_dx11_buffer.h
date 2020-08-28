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
// DirectX11 buffer.
class PlatformBuffer
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformBuffer(ID3D11Buffer* buffer_ptr,
	               ID3D11UnorderedAccessView* uav_ptr,
	               ID3D11ShaderResourceView* srv_ptr);

	// Move constructor.
	PlatformBuffer(PlatformBuffer&&) noexcept;

	// Move operator.
	PlatformBuffer& operator =(PlatformBuffer&&) noexcept;

	// Copying is prohibited.
	PlatformBuffer(const PlatformBuffer&) = delete;
	PlatformBuffer& operator =(const PlatformBuffer&) = delete;

private:
	ut::ComPtr<ID3D11Buffer> d3d11_buffer;
	ut::ComPtr<ID3D11UnorderedAccessView> uav;
	ut::ComPtr<ID3D11ShaderResourceView> srv;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//