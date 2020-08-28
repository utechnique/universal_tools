//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_shader.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor (vertex shader).
PlatformShader::PlatformShader(ID3D11VertexShader* vs_ptr) : vs(vs_ptr)
{}

// Constructor (geometry shader).
PlatformShader::PlatformShader(ID3D11GeometryShader* gs_ptr) : gs(gs_ptr)
{}

// Constructor (hull shader).
PlatformShader::PlatformShader(ID3D11HullShader* hs_ptr) : hs(hs_ptr)
{}

// Constructor (domain shader).
PlatformShader::PlatformShader(ID3D11DomainShader* ds_ptr) : ds(ds_ptr)
{}

// Constructor (pixel shader).
PlatformShader::PlatformShader(ID3D11PixelShader* ps_ptr) : ps(ps_ptr)
{}

// Constructor (compute shader).
PlatformShader::PlatformShader(ID3D11ComputeShader* cs_ptr) : cs(cs_ptr)
{}

// Move constructor.
PlatformShader::PlatformShader(PlatformShader&&) noexcept = default;

// Move operator.
PlatformShader& PlatformShader::operator =(PlatformShader&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//