//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_device.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "FL/x.H" // to get viewport window descriptor
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Creates new DirectX 11 device.
ID3D11Device* CreateDX11Device()
{
	ID3D11Device* new_device_ptr = nullptr;

	UINT create_device_flags = 0;
#ifdef DEBUG
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driver_types[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	const UINT num_driver_types = ARRAYSIZE(driver_types);

	const D3D_FEATURE_LEVEL feature_levels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
	};
	const UINT num_feature_levels = ARRAYSIZE(feature_levels);

	HRESULT result = E_FAIL;
	for (UINT driver_type_id = 0; driver_type_id < num_driver_types; driver_type_id++)
	{
		D3D_DRIVER_TYPE driver_type = driver_types[driver_type_id];
		result = D3D11CreateDevice(nullptr,
			driver_type,
			nullptr,
			create_device_flags,
			feature_levels,
			num_feature_levels,
			D3D11_SDK_VERSION,
			&new_device_ptr,
			nullptr,
			nullptr);

		if (SUCCEEDED(result))
		{
			break;
		}
	}

	if (FAILED(result))
	{
		throw ut::Error(ut::error::fail, ut::Print(result) + " failed to create dx11 device.");
	}

	return new_device_ptr;
}

//----------------------------------------------------------------------------//
// Constructor.
PlatformDevice::PlatformDevice(ID3D11Device* device_ptr) : d3d11_device(device_ptr)
{
	// create dxgi factory
	IDXGIFactory1* gi_factory_ptr;
	HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&gi_factory_ptr));
	if (FAILED(result))
	{
		throw ut::Error(ut::error::fail, ut::Print(result) + " failed to create dxgi factory.");
	}
	gi_factory = ut::ComPtr<IDXGIFactory1>(gi_factory_ptr);
}

// Move constructor.
PlatformDevice::PlatformDevice(PlatformDevice&&) noexcept = default;

// Move operator.
PlatformDevice& PlatformDevice::operator =(PlatformDevice&&) noexcept = default;

// Extracts main context from the dx11 device.
ID3D11DeviceContext* PlatformDevice::GetMainContext()
{
	ID3D11DeviceContext *main_context;
	d3d11_device->GetImmediateContext(&main_context);
	return main_context;
}

// Helper function to conveniently extract both texture and rtv from the backbuffer.
ut::Optional<ut::Error> PlatformDevice::ExtractBackBufferTextureAndView(IDXGISwapChain* swapchain,
	                                                                    ID3D11Texture2D*& texture,
	                                                                    ID3D11RenderTargetView*& view)
{
	// get back buffer texture
	HRESULT result = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture);
	if (FAILED(result))
	{
		return ut::Error(ut::error::fail, ut::Print(result) + " SwapChain->GetBuffer() failed.");
	}

	// create rtv
	result = d3d11_device->CreateRenderTargetView(texture, NULL, &view);
	if (FAILED(result))
	{
		return ut::Error(ut::error::fail, ut::Print(result) + " CreateRenderTargetView() failed.");
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
// Constructor.
Device::Device() : PlatformDevice(CreateDX11Device())
                 , context(GetMainContext())
{}

// Move constructor.
Device::Device(Device&&) noexcept = default;

// Move operator.
Device& Device::operator =(Device&&) noexcept = default;

// Creates new texture.
//    @param width - width of the texture in pixels.
//    @param height - height of the texture in pixels.
//    @return - new texture object of error if failed.
ut::Result<Texture, ut::Error> Device::CreateTexture(pixel::Format format, ut::uint32 width, ut::uint32 height)
{
	ID3D11Texture2D* tex2d = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = ConvertPixelFormatToDX11(format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.BindFlags |= 0;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = d3d11_device->CreateTexture2D(&desc, NULL, &tex2d);
	if (FAILED(hr))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(hr) + " CreateTexture2D failed. "));
	}

	PlatformTexture platform_texture(tex2d, nullptr);

	return Texture(ut::Move(platform_texture), format);
}

// OpenGL doesn't support deferred contexts.
ut::Result<Context, ut::Error> Device::CreateDeferredContext()
{
	ID3D11DeviceContext *deferred_context;
	HRESULT result = d3d11_device->CreateDeferredContext(0, &deferred_context);
	if (FAILED(result))
	{
		return ut::MakeError(ut::error::fail);
	}
	return PlatformContext(deferred_context);
}

// Creates platform-specific representation of the rendering area inside a UI viewport.
//    @param viewport - reference to UI viewport containing rendering area.
//    @return - new display object or error if failed.
ut::Result<Display, ut::Error> Device::CreateDisplay(ui::DesktopViewport& viewport)
{
	// extract windows handle from the viewport widget
	const HWND hwnd = fl_xid(&viewport);

	// get size of the viewport
	const ut::uint32 width = static_cast<ut::uint32>(viewport.w());
	const ut::uint32 height = static_cast<ut::uint32>(viewport.h());

	// initialize swapchain description
	DXGI_SWAP_CHAIN_DESC swapchain_desc;
	ZeroMemory(&swapchain_desc, sizeof(swapchain_desc));
	swapchain_desc.BufferCount = 1;
	swapchain_desc.BufferDesc.Width = width;
	swapchain_desc.BufferDesc.Height = height;
	swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.OutputWindow = hwnd;
	swapchain_desc.Windowed = TRUE;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// create swapchain
	IDXGISwapChain* swapchain;
	HRESULT result = gi_factory->CreateSwapChain(d3d11_device.Get(), &swapchain_desc, &swapchain);
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " CreateSwapChain() failed."));
	}

	// extract texture and rtv from the swapchain to be able to construct ve::render::Target object.
	ID3D11Texture2D* backbuffer;
	ID3D11RenderTargetView* rtv;
	ut::Optional<ut::Error> extract_error = ExtractBackBufferTextureAndView(swapchain, backbuffer, rtv);
	if (extract_error)
	{
		return ut::MakeError(extract_error.Move());
	}

	// create render target that will be associated with provided viewport
	Texture texture(PlatformTexture(backbuffer, nullptr), pixel::r8g8b8a8_srgb);
	Target target(PlatformRenderTarget(rtv, nullptr), ut::Move(texture));

	// success
	return Display(PlatformDisplay(swapchain), ut::Move(target), width, height);
}

// Resizes buffers associated with rendering area inside a UI viewport.
//    @param display - reference to display object.
//    @param width - new width of the display in pixels.
//    @param width - new height of the display in pixels.
//    @return - optional ut::Error if failed.
ut::Optional<ut::Error> Device::ResizeDisplay(Display& display,
                                              ut::uint32 width,
                                              ut::uint32 height)
{
	// release references to backbuffer resources
	display.target.rtv.Delete();
	display.target.buffer.tex2d.Delete();

	// Create new backbuffer
	HRESULT hr = display.dxgi_swapchain->ResizeBuffers(1, width, height,
	                                                   DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	                                                   DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	if (FAILED(hr))
	{
		return ut::Error(ut::error::fail, ut::Print(hr) + " SwapChain->ResizeBuffers() failed. ");
	}

	// extract texture and rtv from the swapchain to be able to construct ve::render::Target object.
	ID3D11Texture2D* backbuffer;
	ID3D11RenderTargetView* rtv;
	ut::Optional<ut::Error> extract_error = ExtractBackBufferTextureAndView(display.dxgi_swapchain.Get(), backbuffer, rtv);
	if (extract_error)
	{
		return extract_error.Move();
	}

	// save new references
	Texture texture(PlatformTexture(backbuffer, nullptr), pixel::r8g8b8a8_srgb);
	display.target = Target(PlatformRenderTarget(rtv, nullptr), ut::Move(texture));

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//