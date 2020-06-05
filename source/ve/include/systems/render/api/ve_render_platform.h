//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
// In terms of rendering system in VE, 'Platform' means a low-level API, such
// as Vulkan, OpenGL, DirectX11 etc. This header is supposed to only have
// one #include directive with a header file that has all platform-specific
// entities declared (whose names start with 'Platform..').
//----------------------------------------------------------------------------//
#if VE_VULKAN
#include "systems/render/api/vulkan/ve_vulkan.h"
#elif VE_OPENGL
#include "systems/render/api/opengl/ve_opengl.h"
#elif VE_DX11
#include "systems/render/api/dx11/ve_dx11.h"
#endif
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//