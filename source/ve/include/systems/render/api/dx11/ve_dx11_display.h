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
// DirectX 11 display.
class PlatformDisplay 
{
	friend class Context;
	friend class Device;
public:
	// Constructor.
	PlatformDisplay(IDXGISwapChain* swapchain_ptr);

	// Move constructor.
	PlatformDisplay(PlatformDisplay&& other) noexcept;

	// Move operator.
	PlatformDisplay& operator =(PlatformDisplay&&) noexcept;

	// Copying is prohibited.
	PlatformDisplay(const PlatformDisplay&) = delete;
	PlatformDisplay& operator =(const PlatformDisplay&) = delete;

private:
	ut::ComPtr<IDXGISwapChain> dxgi_swapchain;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//