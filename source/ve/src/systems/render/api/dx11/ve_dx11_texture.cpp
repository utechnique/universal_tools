//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_texture.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor, accepts 1d texture.
PlatformTexture::PlatformTexture(ID3D11Texture1D* t1d_ptr,
                                 ID3D11ShaderResourceView* srv_ptr) : tex1d(t1d_ptr)
                                                                    , srv(srv_ptr)
{}

// Constructor, accepts 2d texture.
PlatformTexture::PlatformTexture(ID3D11Texture2D* t2d_ptr,
                                 ID3D11ShaderResourceView* srv_ptr) : tex2d(t2d_ptr)
                                                                    , srv(srv_ptr)
{}

// Constructor, accepts 3d texture.
PlatformTexture::PlatformTexture(ID3D11Texture3D* t3d_ptr,
                                 ID3D11ShaderResourceView* srv_ptr) : tex3d(t3d_ptr)
                                                                    , srv(srv_ptr)
{}

// Move constructor.
PlatformTexture::PlatformTexture(PlatformTexture&&) noexcept = default;

// Move operator.
PlatformTexture& PlatformTexture::operator =(PlatformTexture&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//