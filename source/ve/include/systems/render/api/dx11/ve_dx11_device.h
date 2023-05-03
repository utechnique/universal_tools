//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "ut.h"
#include <d3d11.h>
#include <dxgi.h>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// DirectX11 Device.
class PlatformDevice
{
public:
	// Constructor
	PlatformDevice(const ut::String& gpu_name);

	// Move constructor.
	PlatformDevice(PlatformDevice&&) noexcept;

	// Move operator.
	PlatformDevice& operator =(PlatformDevice&&) noexcept;

	// Copying is prohibited.
	PlatformDevice(const PlatformDevice&) = delete;
	PlatformDevice& operator =(const PlatformDevice&) = delete;

protected:
	// Extracts main context from the dx11 device.
	ID3D11DeviceContext* GetMainContext();

	// Helper function to conveniently extract both texture and rtv from the backbuffer.
	ut::Optional<ut::Error> ExtractBackBufferTextureAndView(IDXGISwapChain* swapchain,
	                                                        ID3D11Texture2D*& texture,
	                                                        ID3D11RenderTargetView*& view);
	
	ut::ComPtr<IDXGIFactory1> gi_factory;
	ut::ComPtr<ID3D11Device> d3d11_device;
	ut::ComPtr<ID3D11DeviceContext> immediate_context;
};
//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//