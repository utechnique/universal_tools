//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_buffer.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformBuffer::PlatformBuffer(ID3D11Buffer* buffer_ptr,
                               ID3D11UnorderedAccessView* uav_ptr,
                               ID3D11ShaderResourceView* srv_ptr) : d3d11_buffer(buffer_ptr)
                                                                  , uav(uav_ptr)
                                                                  , srv(srv_ptr)
{}

// Move constructor.
PlatformBuffer::PlatformBuffer(PlatformBuffer&&) noexcept = default;

// Move operator.
PlatformBuffer& PlatformBuffer::operator =(PlatformBuffer&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//