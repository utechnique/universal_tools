//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_device.h"
//----------------------------------------------------------------------------//
#if VE_DX11
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
                                                         , immediate_context(ut::MakeUnique<Context>(GetMainContext()))
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
Device::Device(ut::SharedPtr<ui::Frontend::Thread> ui_frontend) : PlatformDevice(CreateDX11Device())
{}

// Move constructor.
Device::Device(Device&&) noexcept = default;

// Move operator.
Device& Device::operator =(Device&&) noexcept = default;

// Creates new texture.
//    @param info - reference to the ImageInfo object describing an image.
//    @return - new image object of error if failed.
ut::Result<Image, ut::Error> Device::CreateImage(const ImageInfo& info)
{
	ID3D11Texture2D* tex2d = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = info.width;
	desc.Height = info.height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = ConvertPixelFormatToDX11(info.format);
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

	PlatformImage platform_img(tex2d, nullptr);

	return Image(ut::Move(platform_img), info);
}

// Creates platform-specific representation of the rendering area inside a UI viewport.
//    @param viewport - reference to UI viewport containing rendering area.
//    @param vsync - boolean whether to enable vertical synchronization or not.
//    @return - new display object or error if failed.
ut::Result<Display, ut::Error> Device::CreateDisplay(ui::DesktopViewport& viewport, bool vsync)
{
	// extract windows handle from the viewport widget
	const HWND hwnd = fl_xid(&viewport);

	// get size of the viewport
	const ut::uint32 width = static_cast<ut::uint32>(viewport.w());
	const ut::uint32 height = static_cast<ut::uint32>(viewport.h());

	// initialize target backbuffer info
	ImageInfo backbuffer_info;
	backbuffer_info.format = pixel::r8g8b8a8_srgb;
	backbuffer_info.width = width;
	backbuffer_info.height = height;

	// initialize swapchain description
	DXGI_SWAP_CHAIN_DESC swapchain_desc;
	ZeroMemory(&swapchain_desc, sizeof(swapchain_desc));
	swapchain_desc.BufferCount = 1;
	swapchain_desc.BufferDesc.Width = width;
	swapchain_desc.BufferDesc.Height = height;
	swapchain_desc.BufferDesc.Format = ConvertPixelFormatToDX11(backbuffer_info.format);
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

	// initialize render target info
	RenderTargetInfo target_info;
	target_info.usage = RenderTargetInfo::usage_present;

	// create render target that will be associated with provided viewport
	Image texture(PlatformImage(backbuffer, nullptr), backbuffer_info);
	Target target(PlatformRenderTarget(rtv, nullptr), ut::Move(texture), target_info);

	// dx11 does swapping manually, so there is only one buffer available for engine
	ut::Array<Target> display_targets;
	if (!display_targets.Add(ut::Move(target)))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// success
	return Display(PlatformDisplay(swapchain), ut::Move(display_targets), width, height, vsync);
}

// Creates an empty command buffer.
//    @param cmd_buffer_info - reference to the information about
//                             the command buffer to be created.
//    @return - new command buffer or error if failed.
ut::Result<CmdBuffer, ut::Error> Device::CreateCmdBuffer(const CmdBufferInfo& cmd_buffer_info)
{
	return CmdBuffer(PlatformCmdBuffer(), cmd_buffer_info);
}

// Creates render pass object.
//    @param in_color_slots - array of color slots.
//    @param in_depth_stencil_slot - optional depth stencil slot.
//    @return - new render pass or error if failed.
ut::Result<RenderPass, ut::Error> Device::CreateRenderPass(ut::Array<RenderTargetSlot> in_color_slots,
                                                           ut::Optional<RenderTargetSlot> in_depth_stencil_slot)
{
	return RenderPass(PlatformRenderPass(), ut::Move(in_color_slots), ut::Move(in_depth_stencil_slot));
}

// Creates framebuffer. All targets must have the same width and height.
//    @param render_pass - const reference to the renderpass to be bound to.
//    @param color_targets - array of references to the colored render
//                           targets to be bound to a render pass.
//    @param depth_stencil_target - optional reference to the depth-stencil
//                                  target to be bound to a render pass.
//    @return - new framebuffer or error if failed.
ut::Result<Framebuffer, ut::Error> Device::CreateFramebuffer(const RenderPass& render_pass,
	                                                         ut::Array< ut::Ref<Target> > color_targets,
	                                                         ut::Optional<Target&> depth_stencil_target)
{
	// determine width and heights of the framebuffer in pixels
	ut::uint32 width;
	ut::uint32 height;
	if (color_targets.GetNum() != 0)
	{
		const Target& color_target = color_targets.GetFirst();
		width = color_target.image.GetInfo().width;
		height = color_target.image.GetInfo().height;
	}
	else if (depth_stencil_target)
	{
		const Target& ds_target = depth_stencil_target.Get();
		width = ds_target.image.GetInfo().width;
		height = ds_target.image.GetInfo().height;
	}
	else
	{
		return ut::MakeError(ut::Error(ut::error::invalid_arg, "DirectX 11: no targets for the framebuffer."));
	}

	// check width and height of the color targets
	const size_t color_target_count = color_targets.GetNum();
	for (size_t i = 0; i < color_target_count; i++)
	{
		const ImageInfo& img_info = color_targets[i]->image.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "DirectX 11: different width/height for the framebuffer."));
		}
	}

	// check width and height of the depth target
	if (depth_stencil_target)
	{
		const ImageInfo& img_info = depth_stencil_target->image.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "DirectX 11: different width/height for the framebuffer."));
		}
	}

	// initialize info
	FramebufferInfo info(width, height);

	// success
	return Framebuffer(PlatformFramebuffer(), info, ut::Move(color_targets), ut::Move(depth_stencil_target));
}

// Resets all command buffers created with CmdBufferInfo::usage_once flag
// enabled. This call is required before re-recording such command buffers.
// Usualy it's called once per frame.
void Device::ResetDynamicCmdPool()
{}

// Resets given command buffer. This command buffer must be created without
// CmdBufferInfo::usage_once flag (use ResetCmdPool() instead).
//    @param cmd_buffer - reference to the buffer to be reset.
void Device::ResetCmdBuffer(CmdBuffer& cmd_buffer)
{}

// Records commands by calling a provided function.
//    @param cmd_buffer - reference to the buffer to record commands to.
//    @param function - function containing commands to record.
//    @param render_pass - optional reference to the current renderpass,
//                         this parameter applies only for secondary buffer
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
//    @param framebuffer - optional reference to the current framebuffer,
//                         this parameter applies only for secondary buffer
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
void Device::Record(CmdBuffer& cmd_buffer,
	                ut::Function<void(Context&)> function,
	                ut::Optional<RenderPass&> render_pass,
	                ut::Optional<Framebuffer&> framebuffer)
{
	cmd_buffer.proc = function;
}

// Submits a command buffer to a queue. Also it's possible to enqueue presentation
// for displays used in the provided command buffer. Presentation is supported
// only for command buffers created with CmdBufferInfo::usage_once flag.
//    @param cmd_buffer - reference to the command buffer to enqueue.
//    @param present_queue - array of references to displays waiting for their
//                           buffer to be presented to user. Pass empty array
//                           if @cmd_buffer has no CmdBufferInfo::usage_once flag.
void Device::Submit(CmdBuffer& cmd_buffer,
	                ut::Array< ut::Ref<Display> > present_queue)
{
	if (!cmd_buffer.proc)
	{
		throw ut::Error(ut::error::invalid_arg, "DirectX 11: command buffer has no recorded commands.");
	}

	// execute recorded function
	ut::Function<void(Context&)>& procedure = cmd_buffer.proc.Get();
	procedure(immediate_context.GetRef());

	// present
	const size_t present_count = present_queue.GetNum();
	for (size_t i = 0; i < present_count; i++)
	{
		Display& display = present_queue[i].Get();
		display.dxgi_swapchain->Present(display.vsync ? 1 : 0, 0);
	}
}

// Call this function to wait on the host for the completion of all
// queue operations for all queues on this device.
void Device::WaitIdle()
{
	// dx11 manages resources safely, no need to wait
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//