//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_display.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformDisplay::PlatformDisplay(IDXGISwapChain* swapchain_ptr) : dxgi_swapchain(swapchain_ptr)
{}

// Move constructor.
PlatformDisplay::PlatformDisplay(PlatformDisplay&&) noexcept = default;

// Move operator.
PlatformDisplay& PlatformDisplay::operator =(PlatformDisplay&&) noexcept = default;

// Presents a rendered image to the user.
//    @param vsync - 'true' to enable vertical synchronization.
void Display::Present(bool vsync)
{
	dxgi_swapchain->Present(vsync ? 1 : 0, 0);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//