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
// DirectX 11 renderpass.
class PlatformRenderPass
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformRenderPass();

	// Move constructor.
	PlatformRenderPass(PlatformRenderPass&&) noexcept;

	// Move operator.
	PlatformRenderPass& operator =(PlatformRenderPass&&) noexcept;

	// Copying is prohibited.
	PlatformRenderPass(const PlatformRenderPass&) = delete;
	PlatformRenderPass& operator =(const PlatformRenderPass&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//