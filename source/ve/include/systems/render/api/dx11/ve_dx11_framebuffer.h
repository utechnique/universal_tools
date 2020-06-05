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
// DirectX 11 framebuffer.
class PlatformFramebuffer
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformFramebuffer();

	// Move constructor.
	PlatformFramebuffer(PlatformFramebuffer&&) noexcept;

	// Move operator.
	PlatformFramebuffer& operator =(PlatformFramebuffer&&) noexcept;

	// Copying is prohibited.
	PlatformFramebuffer(const PlatformFramebuffer&) = delete;
	PlatformFramebuffer& operator =(const PlatformFramebuffer&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//