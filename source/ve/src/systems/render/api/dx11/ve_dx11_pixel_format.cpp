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
	case pixel::r8g8b8a8_unorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case pixel::b8g8r8a8_unorm: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case pixel::r8g8b8a8_srgb: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case pixel::b8g8r8a8_srgb: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case pixel::r16_float: return DXGI_FORMAT_R16_FLOAT;
	case pixel::r16_sint: return DXGI_FORMAT_R16_SINT;
	case pixel::r16_uint: return DXGI_FORMAT_R16_UINT;
	case pixel::r16_snorm: return DXGI_FORMAT_R16_SNORM;
	case pixel::r16_unorm: return DXGI_FORMAT_R16_UNORM;
	case pixel::r16g16_float: return DXGI_FORMAT_R16G16_FLOAT;
	case pixel::r16g16_sint: return DXGI_FORMAT_R16G16_SINT;
	case pixel::r16g16_uint: return DXGI_FORMAT_R16G16_UINT;
	case pixel::r16g16_snorm: return DXGI_FORMAT_R16G16_SNORM;
	case pixel::r16g16_unorm: return DXGI_FORMAT_R16G16_UNORM;
	case pixel::r16g16b16a16_float: return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case pixel::r16g16b16a16_sint: return DXGI_FORMAT_R16G16B16A16_SINT;
	case pixel::r16g16b16a16_uint: return DXGI_FORMAT_R16G16B16A16_UINT;
	case pixel::r16g16b16a16_snorm: return DXGI_FORMAT_R16G16B16A16_SNORM;
	case pixel::r16g16b16a16_unorm: return DXGI_FORMAT_R16G16B16A16_UNORM;
	case pixel::r32_float: return DXGI_FORMAT_R32_FLOAT;
	case pixel::r32_sint: return DXGI_FORMAT_R32_SINT;
	case pixel::r32_uint: return DXGI_FORMAT_R32_UINT;
	case pixel::r32g32_float: return DXGI_FORMAT_R32G32_FLOAT;
	case pixel::r32g32_sint: return DXGI_FORMAT_R32G32_SINT;
	case pixel::r32g32_uint: return DXGI_FORMAT_R32G32_UINT;
	case pixel::r32g32b32_float: return DXGI_FORMAT_R32G32B32_FLOAT;
	case pixel::r32g32b32_sint: return DXGI_FORMAT_R32G32B32_SINT;
	case pixel::r32g32b32_uint: return DXGI_FORMAT_R32G32B32_UINT;
	case pixel::r32g32b32a32_float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case pixel::r32g32b32a32_sint: return DXGI_FORMAT_R32G32B32A32_SINT;
	case pixel::r32g32b32a32_uint: return DXGI_FORMAT_R32G32B32A32_UINT;
	case pixel::d16_unorm: return DXGI_FORMAT_D16_UNORM;
	case pixel::d24_unorm_s8_uint: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case pixel::d32_float: return DXGI_FORMAT_D32_FLOAT;
	case pixel::d32_float_s8_uint: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	}
	return DXGI_FORMAT_UNKNOWN;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//