//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "ut.h"
#include <d3d11.h>
#include "systems/render/api/ve_render_pixel_format.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts pixel format to the one compatible with DirectX 11.
DXGI_FORMAT ConvertPixelFormatToDX11(pixel::Format format);

// Converts depth pixel format to the one compatible
// with DirectX 11 texture resource.
DXGI_FORMAT ConvertTexDepthPixelFormatToDX11(pixel::Format format);

// Converts depth pixel format to the one compatible
// with DirectX 11 shader resource view.
DXGI_FORMAT ConvertSrvDepthPixelFormatToDX11(pixel::Format format);
//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//