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
// DirectX11 shader.
class PlatformShader
{
	friend class Device;
	friend class Context;
public:
	// Constructor (vertex shader).
	PlatformShader(ID3D11VertexShader* vs_ptr);

	// Constructor (geometry shader).
	PlatformShader(ID3D11GeometryShader* gs_ptr);

	// Constructor (hull shader).
	PlatformShader(ID3D11HullShader* hs_ptr);

	// Constructor (domain shader).
	PlatformShader(ID3D11DomainShader* ds_ptr);

	// Constructor (pixel shader).
	PlatformShader(ID3D11PixelShader* ps_ptr);

	// Constructor (compute shader).
	PlatformShader(ID3D11ComputeShader* cs_ptr);

	// Move constructor.
	PlatformShader(PlatformShader&&) noexcept;

	// Move operator.
	PlatformShader& operator =(PlatformShader&&) noexcept;

	// Copying is prohibited.
	PlatformShader(const PlatformShader&) = delete;
	PlatformShader& operator =(const PlatformShader&) = delete;

private:
	ut::ComPtr<ID3D11VertexShader> vs;
	ut::ComPtr<ID3D11GeometryShader> gs;
	ut::ComPtr<ID3D11HullShader> hs;
	ut::ComPtr<ID3D11DomainShader> ds;
	ut::ComPtr<ID3D11PixelShader> ps;
	ut::ComPtr<ID3D11ComputeShader> cs;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//