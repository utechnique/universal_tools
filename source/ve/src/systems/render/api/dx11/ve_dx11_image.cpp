//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_image.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor, accepts 1d texture.
PlatformImage::PlatformImage(ID3D11Texture1D* t1d_ptr,
                             ID3D11Texture1D* t1d_staging_ptr,
                             ID3D11ShaderResourceView* srv_ptr) : tex1d(t1d_ptr)
                                                                , tex1d_staging(t1d_staging_ptr)
                                                                , srv(srv_ptr)
{}

// Constructor, accepts 2d texture.
PlatformImage::PlatformImage(ID3D11Texture2D* t2d_ptr,
                             ID3D11Texture2D* t2d_staging_ptr,
                             ID3D11ShaderResourceView* srv_ptr,
                             ID3D11ShaderResourceView** cube_faces_ptr) : tex2d(t2d_ptr)
                                                                        , tex2d_staging(t2d_staging_ptr)
                                                                        , srv(srv_ptr)
{
	if (cube_faces_ptr)
	{
		for (ut::uint32 i = 0; i < 6; i++)
		{
			ut::ComPtr<ID3D11ShaderResourceView> face(cube_faces_ptr[i]);
			cube_faces[i] = ut::Move(face);
		}
	}
}

// Constructor, accepts 3d texture.
PlatformImage::PlatformImage(ID3D11Texture3D* t3d_ptr,
                             ID3D11Texture3D* t3d_staging_ptr,
                             ID3D11ShaderResourceView* srv_ptr) : tex3d(t3d_ptr)
                                                                , tex3d_staging(t3d_staging_ptr)
                                                                , srv(srv_ptr)
{}

// Move constructor.
PlatformImage::PlatformImage(PlatformImage&&) noexcept = default;

// Move operator.
PlatformImage& PlatformImage::operator =(PlatformImage&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//