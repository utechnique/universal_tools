//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// OpenGL render target.
class PlatformRenderTarget
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformRenderTarget();

	// Move constructor.
	PlatformRenderTarget(PlatformRenderTarget&&) noexcept;

	// Move operator.
	PlatformRenderTarget& operator =(PlatformRenderTarget&&) noexcept;

	// Copying is prohibited.
	PlatformRenderTarget(const PlatformRenderTarget&) = delete;
	PlatformRenderTarget& operator =(const PlatformRenderTarget&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//