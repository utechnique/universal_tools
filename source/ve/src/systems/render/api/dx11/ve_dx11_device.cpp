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

// Converts usage to the one compatible with DirectX11.
D3D11_USAGE ConvertUsageToDX11(memory::Usage usage)
{
	switch (usage)
	{
	case render::memory::gpu:       return D3D11_USAGE_DEFAULT;
	case render::memory::gpu_cpu:   return D3D11_USAGE_DYNAMIC;
	case render::memory::immutable: return D3D11_USAGE_IMMUTABLE;
	}
	return D3D11_USAGE_DEFAULT;
}

// Converts compare operation to the one compatible with DirectX11.
D3D11_COMPARISON_FUNC ConvertCompareOpToDX11(compare::Operation op)
{
	switch (op)
	{
	case compare::never:            return D3D11_COMPARISON_NEVER;
	case compare::less:             return D3D11_COMPARISON_LESS;
	case compare::equal:            return D3D11_COMPARISON_EQUAL;
	case compare::less_or_equal:    return D3D11_COMPARISON_LESS_EQUAL;
	case compare::greater:          return D3D11_COMPARISON_GREATER;
	case compare::not_equal:        return D3D11_COMPARISON_NOT_EQUAL;
	case compare::greater_or_equal: return D3D11_COMPARISON_GREATER_EQUAL;
	case compare::always:           return D3D11_COMPARISON_ALWAYS;
	}
	return D3D11_COMPARISON_ALWAYS;
}

// Converts stencil operation to the one compatible with DirectX11.
D3D11_STENCIL_OP ConvertStencilOpToDX11(StencilOpState::Operation op)
{
	switch (op)
	{
	case StencilOpState::keep:                return D3D11_STENCIL_OP_KEEP;
	case StencilOpState::zero:                return D3D11_STENCIL_OP_ZERO;
	case StencilOpState::replace:             return D3D11_STENCIL_OP_REPLACE;
	case StencilOpState::increment_and_clamp: return D3D11_STENCIL_OP_INCR_SAT;
	case StencilOpState::decrement_and_clamp: return D3D11_STENCIL_OP_DECR_SAT;
	case StencilOpState::invert:              return D3D11_STENCIL_OP_INVERT;
	case StencilOpState::increment_and_wrap:  return D3D11_STENCIL_OP_INCR;
	case StencilOpState::decrement_and_wrap:  return D3D11_STENCIL_OP_DECR;
	}
	return D3D11_STENCIL_OP_KEEP;
}

// Converts cull mode to the one compatible with DirectX11.
D3D11_CULL_MODE ConvertCullModeToDX11(RasterizationState::CullMode mode)
{
	switch (mode)
	{
	case RasterizationState::no_culling:    return D3D11_CULL_NONE;
	case RasterizationState::front_culling: return D3D11_CULL_FRONT;
	case RasterizationState::back_culling:  return D3D11_CULL_BACK;
	}
	return D3D11_CULL_NONE;
}

// Converts blend operation to the one compatible with DirectX11.
D3D11_BLEND_OP ConvertBlendOpToDX11(Blending::Operation op)
{
	switch (op)
	{
	case Blending::add:              return D3D11_BLEND_OP_ADD;
	case Blending::subtract:         return D3D11_BLEND_OP_SUBTRACT;
	case Blending::reverse_subtract: return D3D11_BLEND_OP_REV_SUBTRACT;
	case Blending::min:              return D3D11_BLEND_OP_MIN;
	case Blending::max:              return D3D11_BLEND_OP_MAX;
	}
	return D3D11_BLEND_OP_MAX;
}

// Converts blend factor to the one compatible with DirectX11.
D3D11_BLEND ConvertBlendFactorToDX11(Blending::Factor op)
{
	switch (op)
	{
	case Blending::zero:                return D3D11_BLEND_ZERO;
	case Blending::one:                 return D3D11_BLEND_ONE;
	case Blending::src_color:           return D3D11_BLEND_SRC_COLOR;
	case Blending::inverted_src_color:  return D3D11_BLEND_INV_SRC_COLOR;
	case Blending::dst_color:           return D3D11_BLEND_DEST_COLOR;
	case Blending::inverted_dst_color:  return D3D11_BLEND_INV_DEST_COLOR;
	case Blending::src_alpha:           return D3D11_BLEND_SRC_ALPHA;
	case Blending::inverted_src_alpha:  return D3D11_BLEND_INV_SRC_ALPHA;
	case Blending::dst_alpha:           return D3D11_BLEND_DEST_ALPHA;
	case Blending::inverted_dst_alpha:  return D3D11_BLEND_INV_DEST_ALPHA;
	case Blending::src1_color:          return D3D11_BLEND_SRC1_COLOR;
	case Blending::inverted_src1_color: return D3D11_BLEND_INV_SRC1_COLOR;
	case Blending::src1_alpha:          return D3D11_BLEND_SRC1_ALPHA;
	case Blending::inverted_src1_alpha: return D3D11_BLEND_INV_SRC1_ALPHA;
	}
	return D3D11_BLEND_ONE;
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
ut::Result<CmdBuffer, ut::Error> Device::CreateCmdBuffer(const CmdBuffer::Info& cmd_buffer_info)
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

// Creates a buffer.
//    @param info - ve::render::Buffer::Info object to initialize a buffer with.
//    @return - new shader or error if failed.
ut::Result<Buffer, ut::Error> Device::CreateBuffer(Buffer::Info info)
{
	ID3D11Buffer *buffer = nullptr;
	ID3D11UnorderedAccessView *uav = nullptr;
	ID3D11ShaderResourceView *srv = nullptr;
	D3D11_BUFFER_DESC buffer_desc;

	// usage, length, misc
	buffer_desc.ByteWidth = static_cast<UINT>(info.size);
	buffer_desc.Usage = ConvertUsageToDX11(info.usage);
	buffer_desc.MiscFlags = 0;

	// bind flags
	switch (info.type)
	{
	case Buffer::vertex:  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   break;
	case Buffer::index:   buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;    break;
	case Buffer::uniform: buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break;
	case Buffer::storage: buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE; break;
	}

	// cpu access
	if (info.usage == render::memory::gpu_cpu)
	{
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		buffer_desc.CPUAccessFlags = 0;
	}

	if (info.type == Buffer::storage)
	{
		buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		buffer_desc.StructureByteStride = info.stride;
	}

	// init data
	D3D11_SUBRESOURCE_DATA subrc_data;
	D3D11_SUBRESOURCE_DATA *init_data;
	if (!info.data.IsEmpty())
	{
		init_data = &subrc_data;
		subrc_data.pSysMem = info.data.GetAddress();
	}
	else
	{
		init_data = nullptr;
	}

	// create DX11 buffer
	HRESULT result = d3d11_device->CreateBuffer(&buffer_desc, init_data, &buffer);
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 buffer."));
	}

	// create DX11 uav and srv
	if (info.type == Buffer::storage)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc_uav;
		D3D11_SHADER_RESOURCE_VIEW_DESC  desc_srv;

		desc_uav.Format = DXGI_FORMAT_UNKNOWN;
		desc_uav.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc_uav.Buffer.FirstElement = 0;
		desc_uav.Buffer.NumElements = static_cast<UINT>(info.size / info.stride);
		desc_uav.Buffer.Flags = 0;

		desc_srv.Format = DXGI_FORMAT_UNKNOWN;
		desc_srv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc_srv.Buffer.FirstElement = 0;
		desc_srv.Buffer.NumElements = static_cast<UINT>(info.size / info.stride);

		result = d3d11_device->CreateUnorderedAccessView(buffer, &desc_uav, &uav);
		if (FAILED(result))
		{
			return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 buffer UAV."));
		}

		result = d3d11_device->CreateShaderResourceView(buffer, &desc_srv, &srv);
		if (FAILED(result))
		{
			return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 buffer srv."));
		}
	}

	// success
	PlatformBuffer platform_buffer(buffer, uav, srv);
	return Buffer(ut::Move(platform_buffer), ut::Move(info));
}

// Creates a shader.
//    @param info - ve::render::Shader::Info object to initialize a shader with.
//    @return - new shader or error if failed.
ut::Result<Shader, ut::Error> Device::CreateShader(Shader::Info info)
{
	ID3D11VertexShader*   vs = nullptr;
	ID3D11GeometryShader* gs = nullptr;
	ID3D11HullShader*     hs = nullptr;
	ID3D11DomainShader*   ds = nullptr;
	ID3D11PixelShader*    ps = nullptr;
	ID3D11ComputeShader*  cs = nullptr;

	// create DX11 shader
	switch (info.stage)
	{
		case Shader::vertex:
		{
			HRESULT result = d3d11_device->CreateVertexShader(info.bytecode.GetAddress(),
															  info.bytecode.GetSize(),
															  nullptr,
															  &vs);
			if (FAILED(result))
			{
				return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 vertex shader."));
			}
		} break;

		case Shader::hull:
		{
			HRESULT result = d3d11_device->CreateHullShader(info.bytecode.GetAddress(),
															info.bytecode.GetSize(),
															nullptr,
															&hs);
			if (FAILED(result))
			{
				return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 hull shader."));
			}
		} break;

		case Shader::domain:
		{
			HRESULT result = d3d11_device->CreateDomainShader(info.bytecode.GetAddress(),
															  info.bytecode.GetSize(),
															  nullptr,
															  &ds);
			if (FAILED(result))
			{
				return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 domain shader."));
			}
		} break;

		case Shader::geometry:
		{
			HRESULT result = d3d11_device->CreateGeometryShader(info.bytecode.GetAddress(),
																info.bytecode.GetSize(),
																nullptr,
																&gs);
			if (FAILED(result))
			{
				return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 geometry shader."));
			}
		} break;

		case Shader::pixel:
		{
			HRESULT result = d3d11_device->CreatePixelShader(info.bytecode.GetAddress(),
															 info.bytecode.GetSize(),
															 nullptr,
															 &ps);
			if (FAILED(result))
			{
				return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 pixel shader."));
			}
		} break;

		case Shader::compute:
		{
			HRESULT result = d3d11_device->CreateComputeShader(info.bytecode.GetAddress(),
															   info.bytecode.GetSize(),
															   nullptr,
															   &cs);
			if (FAILED(result))
			{
				return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 compute shader."));
			}
		} break;
	}

	// create ve shader
	switch (info.stage)
	{
	case Shader::vertex:   return Shader(PlatformShader(vs), ut::Move(info));
	case Shader::hull:     return Shader(PlatformShader(hs), ut::Move(info));
	case Shader::domain:   return Shader(PlatformShader(ds), ut::Move(info));
	case Shader::geometry: return Shader(PlatformShader(gs), ut::Move(info));
	case Shader::pixel:    return Shader(PlatformShader(ps), ut::Move(info));
	case Shader::compute:  return Shader(PlatformShader(cs), ut::Move(info));
	}
	return ut::MakeError(ut::error::fail);
}

// Creates a pipeline state.
//    @param info - ve::render::PipelineState::Info object to
//                  initialize a pipeline with.
//    @param render_pass - reference to the ve::render::RenderPass object
//                         pipeline will be bound to.
//    @return - new pipeline sate or error if failed.
ut::Result<PipelineState, ut::Error> Device::CreatePipelineState(PipelineState::Info info,
                                                                 RenderPass& render_pass)
{
	HRESULT result;
	ID3D11InputLayout* input_layout = nullptr;
	ID3D11BlendState* blend_state = nullptr;
	ID3D11RasterizerState* rasterizer_state = nullptr;
	ID3D11DepthStencilState* depthstencil_state = nullptr;

	// descriptors of dx11 input layout elements
	const UINT input_element_count = static_cast<UINT>(info.input_assembly_state.elements.GetNum());
	ut::Array<D3D11_INPUT_ELEMENT_DESC> input_el_desc(input_element_count);
	for (UINT i = 0; i < input_element_count; i++)
	{
		VertexElement& element = info.input_assembly_state.elements[i];

		D3D11_INPUT_ELEMENT_DESC& desc = input_el_desc[i];
		desc.SemanticName = element.semantic_name;
		desc.SemanticIndex = element.semantic_id;
		desc.Format = ConvertPixelFormatToDX11(element.format);
		desc.InputSlot = 0;
		desc.AlignedByteOffset = element.offset;
		desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = 0;
	}

	// create dx11 input layout
	if (info.stages[Shader::vertex])
	{
		const ut::Array<ut::byte>& vs_bytecode = info.stages[Shader::vertex]->info.bytecode;

		result = d3d11_device->CreateInputLayout(input_el_desc.GetAddress(),
		                                         input_element_count,
		                                         vs_bytecode.GetAddress(),
		                                         vs_bytecode.GetSize(),
		                                         &input_layout);
		if (FAILED(result))
		{
			return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 input layout."));
		}
	}
	else
	{
		ut::log.Lock() << "Warning! Pipeline state has no vertex shader stage." << ut::cret;
	}

	// check if at least one scissor is used
	const size_t viewport_count = info.viewports.GetNum();
	BOOL scissor_enable = FALSE;
	for (size_t i = 0; i < viewport_count; i++)
	{
		if (info.viewports[i].scissor)
		{
			scissor_enable = TRUE;
			break;
		}
	}

	// create rasterizer state
	const RasterizationState& rs = info.rasterization_state;
	D3D11_RASTERIZER_DESC raster_desc;
	raster_desc.FillMode = rs.polygon_mode == RasterizationState::fill ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
	raster_desc.CullMode = ConvertCullModeToDX11(rs.cull_mode);
	raster_desc.FrontCounterClockwise = rs.front_face == RasterizationState::counter_clockwise ? TRUE : FALSE;
	raster_desc.DepthBias = rs.depth_bias_enable ? 1 : 0;
	raster_desc.DepthBiasClamp = rs.depth_bias_clamp;
	raster_desc.SlopeScaledDepthBias = rs.depth_bias_slope_factor;
	raster_desc.DepthClipEnable = TRUE;
	raster_desc.ScissorEnable = scissor_enable;
	raster_desc.MultisampleEnable = FALSE;
	raster_desc.AntialiasedLineEnable = FALSE;
	result = d3d11_device->CreateRasterizerState(&raster_desc, &rasterizer_state);
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 rasterizer state."));
	}

	// create depth-stencil state
	const DepthStencilState& ds = info.depth_stencil_state;
	D3D11_DEPTH_STENCIL_DESC ds_desc;
	ds_desc.DepthEnable = ds.depth_test_enable;
	ds_desc.DepthWriteMask = ds.depth_write_enable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	ds_desc.DepthFunc = ConvertCompareOpToDX11(ds.depth_compare_op);
	ds_desc.StencilEnable = ds.stencil_test_enable;
	ds_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	ds_desc.StencilWriteMask = ds.stencil_write_mask;
	ds_desc.FrontFace.StencilFunc = ConvertCompareOpToDX11(ds.front.compare_op);
	ds_desc.FrontFace.StencilDepthFailOp = ConvertStencilOpToDX11(ds.front.depth_fail_op);
	ds_desc.FrontFace.StencilPassOp = ConvertStencilOpToDX11(ds.front.pass_op);
	ds_desc.FrontFace.StencilFailOp = ConvertStencilOpToDX11(ds.front.fail_op);
	ds_desc.BackFace.StencilFunc = ConvertCompareOpToDX11(ds.back.compare_op);
	ds_desc.BackFace.StencilDepthFailOp = ConvertStencilOpToDX11(ds.back.depth_fail_op);
	ds_desc.BackFace.StencilPassOp = ConvertStencilOpToDX11(ds.back.pass_op);
	ds_desc.BackFace.StencilFailOp = ConvertStencilOpToDX11(ds.back.fail_op);
	result = d3d11_device->CreateDepthStencilState(&ds_desc, &depthstencil_state);
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 depth-stencil state."));
	}

	// check blendstate attachment count
	const size_t blend_attachment_count = info.blend_state.attachments.GetNum();
	if (blend_attachment_count > 8)
	{
		return ut::MakeError(ut::Error(ut::error::fail, "Too many blendstate attachments (max 8 for DX11)."));
	}

	// create blend state
	D3D11_BLEND_DESC blend_desc;
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = TRUE;
	for (size_t i = 0; i < 8; i++)
	{
		D3D11_RENDER_TARGET_BLEND_DESC& target = blend_desc.RenderTarget[i];
		if (i >= blend_attachment_count)
		{
			ut::memory::Set(&target, 0, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
			continue;
		}

		Blending& blending = info.blend_state.attachments[i];
		target.BlendEnable = blending.blend_enable ? TRUE : FALSE;
		target.SrcBlend = ConvertBlendFactorToDX11(blending.src_blend);
		target.DestBlend = ConvertBlendFactorToDX11(blending.dst_blend);
		target.BlendOp = ConvertBlendOpToDX11(blending.color_op);
		target.SrcBlendAlpha = ConvertBlendFactorToDX11(blending.src_alpha);
		target.DestBlendAlpha = ConvertBlendFactorToDX11(blending.dst_alpha);
		target.BlendOpAlpha = ConvertBlendOpToDX11(blending.alpha_op);
		target.RenderTargetWriteMask = blending.write_mask;
	}
	result = d3d11_device->CreateBlendState(&blend_desc, &blend_state);
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to create d3d11 blend state."));
	}

	// success
	PlatformPipelineState platform_pipeline(input_layout,
	                                        rasterizer_state,
	                                        depthstencil_state,
	                                        blend_state);
	return PipelineState(ut::Move(platform_pipeline), ut::Move(info));
}

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

// Waits for all commands from the provided buffer to be executed.
//    @param cmd_buffer - reference to the command buffer.
void Device::WaitCmdBuffer(CmdBuffer& cmd_buffer)
{
	cmd_buffer.pending = false;
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

// Acquires next buffer in a swapchain to be filled.
//    @param display - reference to the display.
void Device::AcquireNextDisplayBuffer(Display& display)
{}

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