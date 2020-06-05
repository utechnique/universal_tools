//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/dx11/ve_dx11_pixel_format.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts pixel format to the one compatible with DirectX 11.
DXGI_FORMAT ConvertPixelFormatToDX11(pixel::Format format)
{
	switch (format)
	{
	case pixel::r8g8b8: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case pixel::b8g8r8: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case pixel::r8g8b8a8: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case pixel::b8g8r8a8: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case pixel::r8g8b8a8_srgb: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case pixel::b8g8r8a8_srgb: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	}
	return DXGI_FORMAT_UNKNOWN;
}

// Converts DirectX 11 pixel format to ve::render::pixel::Format value.
pixel::Format ConvertPixelFormatFromDX11(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM: return pixel::r8g8b8a8;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return pixel::b8g8r8a8;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return pixel::r8g8b8a8_srgb;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return pixel::b8g8r8a8_srgb;
	}
	return pixel::unknown;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//