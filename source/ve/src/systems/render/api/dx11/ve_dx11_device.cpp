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
	case render::memory::gpu_read_write:             return D3D11_USAGE_DEFAULT;
	case render::memory::gpu_read_write_cpu_staging: return D3D11_USAGE_DEFAULT;
	case render::memory::gpu_read_cpu_write:         return D3D11_USAGE_DYNAMIC;
	case render::memory::gpu_immutable:              return D3D11_USAGE_IMMUTABLE;
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

// Converts address mode value to the one compatible with DirectX11.
D3D11_TEXTURE_ADDRESS_MODE ConvertTexAddressModeToDX11(Sampler::AddressMode mode)
{
	switch (mode)
	{
	case Sampler::address_wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
	case Sampler::address_mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
	case Sampler::address_clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
	case Sampler::address_border: return D3D11_TEXTURE_ADDRESS_BORDER;
	}
	return D3D11_TEXTURE_ADDRESS_WRAP;
}

//----------------------------------------------------------------------------//
// Constructor.
PlatformDevice::PlatformDevice(ID3D11Device* device_ptr) : d3d11_device(device_ptr)
                                                         , immediate_context(GetMainContext())
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
{
	info.max_uniform_buffer_size = 65536;
	info.max_storage_buffer_size = 268435456;
	info.max_1D_image_dimension = 16384;
	info.max_2D_image_dimension = 16384;
	info.max_3D_image_dimension = 16384;
	info.max_cube_image_dimension = 16384;
	info.max_image_array_size = 2048;
	info.supports_geometry_shader = true;
	info.supports_tesselation_shader = true;
	info.supports_wide_lines = false;
	info.supports_async_rc_mapping = false;
	info.supports_sv_instance_offset = false;

	for (ut::uint32 i = 0; i < pixel::format_count; i++)
	{
		info.supports_render_target_format[i] = true;
	}
}

// Move constructor.
Device::Device(Device&&) noexcept = default;

// Move operator.
Device& Device::operator =(Device&&) noexcept = default;

// Creates a new texture.
//    @param info - reference to the Image::Info object describing an image.
//    @return - new image object of error if failed.
ut::Result<Image, ut::Error> Device::CreateImage(Image::Info info)
{
	HRESULT result;

	D3D11_SUBRESOURCE_DATA* subrc_data = nullptr;
	ut::Array<D3D11_SUBRESOURCE_DATA> subresources;

	ID3D11Texture1D *tex1D = nullptr;
	ID3D11Texture2D *tex2D = nullptr;
	ID3D11Texture3D *tex3D = nullptr;

	// figure out pixel size
	const ut::uint32 pixel_size = pixel::GetSize(info.format);
	if (pixel_size == 0)
	{
		return ut::MakeError(ut::error::invalid_arg, "DX11: Cannot create image with zero pixel size.");
	}

	// dimensions
	bool is_1d = info.type == Image::type_1D;
	bool is_2d = info.type == Image::type_2D || info.type == Image::type_cube;
	bool is_3d = info.type == Image::type_3D;
	bool is_cube = info.type == Image::type_cube;

	// check if image is used as a render target
	const bool is_render_target = info.usage == render::memory::gpu_read_write;
	const bool is_depth_buffer = is_render_target && pixel::IsDepthFormat(info.format);

	// initialize subresource data
	if (!info.data.IsEmpty())
	{
		ut::byte* mip_data = info.data.GetAddress();

		const ut::uint32 cubeface_count = is_cube ? 6 : 1;

		subresources.Resize(cubeface_count * info.mip_count);
		
		for (ut::uint32 i = 0; i < cubeface_count; i++)
		{
			ut::uint32 mip_width = info.width;
			ut::uint32 mip_height = info.height;
			ut::uint32 mip_depth = info.depth;

			for (ut::uint32 j = 0; j < info.mip_count; j++)
			{
				const ut::uint32 subrc_id = i * info.mip_count + j;

				subresources[subrc_id].pSysMem = mip_data;
				subresources[subrc_id].SysMemPitch = pixel_size * mip_width;
				subresources[subrc_id].SysMemSlicePitch = pixel_size * mip_width * mip_height;

				ut::uint32 mip_size = pixel_size * mip_width;
				mip_width /= 2;

				if (is_2d || is_3d)
				{
					mip_size *= mip_height;
					mip_height /= 2;
				}

				if (is_3d)
				{
					mip_size *= mip_depth;
					mip_depth /= 2;
				}

				mip_data += mip_size;
			}
		}

		subrc_data = subresources.GetAddress();
	}

	// cpu access
	UINT cpu_access = info.usage == render::memory::gpu_read_cpu_write ? D3D11_CPU_ACCESS_WRITE : 0;

	// bind flags
	UINT bind_flags = D3D11_BIND_SHADER_RESOURCE;
	if (is_render_target)
	{
		bind_flags |= is_depth_buffer ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
	}

	// pixel format
	DXGI_FORMAT tex_pixel_format = is_depth_buffer ?
	                               ConvertTexDepthPixelFormatToDX11(info.format) :
	                               ConvertPixelFormatToDX11(info.format);
	DXGI_FORMAT srv_pixel_format = is_depth_buffer ?
	                               ConvertSrvDepthPixelFormatToDX11(info.format) :
	                               tex_pixel_format;

	// shader resource view description
	ID3D11ShaderResourceView* srv = nullptr;
	ID3D11ShaderResourceView* cube_faces[6];
	ID3D11Resource* shader_resource = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
	srv_desc.Format = srv_pixel_format;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = info.mip_count;

	// create appropriate texture
	if (is_1d)
	{
		D3D11_TEXTURE1D_DESC tex1d_desc;
		tex1d_desc.Width = info.width;
		tex1d_desc.MipLevels = info.mip_count;
		tex1d_desc.ArraySize = 1;
		tex1d_desc.Format = tex_pixel_format;
		tex1d_desc.Usage = ConvertUsageToDX11(info.usage);
		tex1d_desc.BindFlags = bind_flags;
		tex1d_desc.CPUAccessFlags = cpu_access;
		tex1d_desc.MiscFlags = 0;
		result = d3d11_device->CreateTexture1D(&tex1d_desc, subrc_data, &tex1D);
		if (FAILED(result))
		{
			return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 1d texture.");
		}

		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		shader_resource = tex1D;
	}
	else if (is_2d)
	{
		D3D11_TEXTURE2D_DESC tex2d_desc;
		tex2d_desc.Width = info.width;
		tex2d_desc.Height = info.height;
		tex2d_desc.MipLevels = info.mip_count;
		tex2d_desc.ArraySize = info.type == Image::type_cube ? 6 : 1;
		tex2d_desc.Format = tex_pixel_format;
		tex2d_desc.SampleDesc.Count = 1;
		tex2d_desc.SampleDesc.Quality = 0;
		tex2d_desc.Usage = ConvertUsageToDX11(info.usage);
		tex2d_desc.BindFlags = bind_flags;
		tex2d_desc.CPUAccessFlags = cpu_access;
		tex2d_desc.MiscFlags = is_cube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
		if (is_render_target && info.mip_count > 1)
		{
			tex2d_desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		result = d3d11_device->CreateTexture2D(&tex2d_desc, subrc_data, &tex2D);
		if (FAILED(result))
		{
			return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 2d texture.");
		}

		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shader_resource = tex2D;

		// create dx11 cube srv if texture is a cubemap
		if (is_cube)
		{
			srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;

			D3D11_SHADER_RESOURCE_VIEW_DESC face_srv_desc = srv_desc;

			face_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			face_srv_desc.Texture2DArray.ArraySize = 1;

			for (int i = 0; i < 6; i++)
			{
				face_srv_desc.Texture2DArray.FirstArraySlice = i;
				result = d3d11_device->CreateShaderResourceView(tex2D, &face_srv_desc, &cube_faces[i]);
				if (FAILED(result))
				{
					return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 srv for a cube face.");
				}
			}
		}
	}
	else if (is_3d)
	{
		D3D11_TEXTURE3D_DESC tex3d_desc;
		tex3d_desc.Width = info.width;
		tex3d_desc.Height = info.height;
		tex3d_desc.Depth = info.depth;
		tex3d_desc.MipLevels = info.mip_count;
		tex3d_desc.Format = tex_pixel_format;
		tex3d_desc.Usage = ConvertUsageToDX11(info.usage);
		tex3d_desc.BindFlags = bind_flags;
		tex3d_desc.CPUAccessFlags = cpu_access;
		tex3d_desc.MiscFlags = 0;
		result = d3d11_device->CreateTexture3D(&tex3d_desc, subrc_data, &tex3D);
		if (FAILED(result))
		{
			return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 3d texture.");
		}

		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		shader_resource = tex3D;
	}

	// create shader resource view
	result = d3d11_device->CreateShaderResourceView(shader_resource, &srv_desc, &srv);
	if (FAILED(result))
	{
		return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 srv for 1d texture.");
	}

	// success
	PlatformImage platform_img = is_1d ? PlatformImage(tex1D, srv) :
	                             is_2d ? PlatformImage(tex2D, srv, is_cube ? cube_faces : nullptr) :
	                             PlatformImage(tex3D, srv);
	return Image(ut::Move(platform_img), ut::Move(info));
}

// Creates a new sampler.
//    @param info - reference to the Sampler::Info object describing a sampler.
//    @return - new sampler object of error if failed.
ut::Result<Sampler, ut::Error> Device::CreateSampler(const Sampler::Info& info)
{
	ID3D11SamplerState* sampler;
	D3D11_SAMPLER_DESC sampler_desc;

	sampler_desc.AddressU = ConvertTexAddressModeToDX11(info.address_u);
	sampler_desc.AddressV = ConvertTexAddressModeToDX11(info.address_v);
	sampler_desc.AddressW = ConvertTexAddressModeToDX11(info.address_w);
	sampler_desc.MipLODBias = info.mip_lod_bias;
	sampler_desc.MaxAnisotropy = static_cast<UINT>(info.max_anisotropy);
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_desc.BorderColor[0] = info.border_color.R();
	sampler_desc.BorderColor[1] = info.border_color.G();
	sampler_desc.BorderColor[2] = info.border_color.B();
	sampler_desc.BorderColor[3] = info.border_color.A();
	sampler_desc.MinLOD = info.min_lod;
	sampler_desc.MaxLOD = info.max_lod;

	if (info.compare_op)
	{
		sampler_desc.ComparisonFunc = ConvertCompareOpToDX11(info.compare_op.Get());
	}

	if (info.mag_filter == Sampler::filter_nearest)
	{
		if (info.min_filter == Sampler::filter_nearest)
		{
			if (info.mip_filter == Sampler::filter_nearest)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			}
			else if (info.mip_filter == Sampler::filter_linear)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			}
		}
		else if (info.min_filter == Sampler::filter_linear)
		{
			if (info.mip_filter == Sampler::filter_nearest)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			}
			else if (info.mip_filter == Sampler::filter_linear)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			}
		}
	}
	else if (info.mag_filter == Sampler::filter_linear)
	{
		if (info.min_filter == Sampler::filter_nearest)
		{
			if (info.mip_filter == Sampler::filter_nearest)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			}
			else if (info.mip_filter == Sampler::filter_linear)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			}
		}
		else if (info.min_filter == Sampler::filter_linear)
		{
			if (info.mip_filter == Sampler::filter_nearest)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			}
			else if (info.mip_filter == Sampler::filter_linear)
			{
				sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}
		}
	}

	if (info.anisotropy_enable)
	{
		sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	}

	HRESULT result = d3d11_device->CreateSamplerState(&sampler_desc, &sampler);
	if (FAILED(result))
	{
		return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 sampler.");
	}

	PlatformSampler platform_sampler(sampler);
	return Sampler(ut::Move(platform_sampler), info);
}

// Creates a new render target.
//    @param info - reference to the Target::Info object describing a target.
//    @return - new render target object of error if failed.
ut::Result<Target, ut::Error> Device::CreateTarget(const Target::Info& info)
{
	// no image other than 2d or cubemap can be a render target
	if (info.type != Image::type_2D && info.type != Image::type_cube)
	{
		return ut::MakeError(ut::error::not_supported, "Only 2D image can be a render target.");
	}

	// create image resource for the render target
	Image::Info img_info;
	img_info.type = info.type;
	img_info.format = info.format;
	img_info.usage = render::memory::gpu_read_write;
	img_info.mip_count = info.mip_count;
	img_info.width = info.width;
	img_info.height = info.height;
	img_info.depth = info.depth;
	ut::Result<Image, ut::Error> image = CreateImage(img_info);
	if (!image)
	{
		return ut::MakeError(image.MoveAlt());
	}

	// retreice 2d texture resource
	ID3D11Texture2D *tex2d = image->tex2d.Get();

	// check if image is a cubemap
	const bool is_cube = info.type == Image::type_cube;

	// create render target views
	const ut::uint32 slice_count = is_cube ? 6 : 1;
	ut::Array<PlatformRenderTarget::SliceRTV> slice_target_views(slice_count);
	for (ut::uint32 slice = 0; slice < slice_count; slice++)
	{
		for (ut::uint32 mip = 0; mip < info.mip_count; mip++)
		{
			ID3D11RenderTargetView* rtv = nullptr;
			ID3D11DepthStencilView* dsv = nullptr;
			if (info.usage == Target::Info::usage_depth)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC desc;
				desc.Format = ConvertPixelFormatToDX11(info.format);
				desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				desc.Flags = 0;
				desc.Texture2D.MipSlice = mip;
				if (is_cube)
				{
					desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.FirstArraySlice = slice;
					desc.Texture2DArray.ArraySize = 1;
					desc.Texture2DArray.MipSlice = mip;
				}

				HRESULT result = d3d11_device->CreateDepthStencilView(tex2d, &desc, &dsv);
				if (FAILED(result))
				{
					return ut::MakeError(ut::error::fail, ut::Print(result) + " CreateDepthStencilView() failed.");
				}
			}
			else
			{
				D3D11_RENDER_TARGET_VIEW_DESC desc;
				desc.Format = ConvertPixelFormatToDX11(info.format);
				desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = mip;
				if (is_cube)
				{
					desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					desc.Texture2DArray.FirstArraySlice = slice;
					desc.Texture2DArray.ArraySize = 1;
					desc.Texture2DArray.MipSlice = mip;
				}

				HRESULT result = d3d11_device->CreateRenderTargetView(tex2d, &desc, &rtv);
				if (FAILED(result))
				{
					return ut::MakeError(ut::error::fail, ut::Print(result) + " CreateRenderTargetView() failed.");
				}
			}

			slice_target_views[slice].mips.Add({ ut::ComPtr<ID3D11RenderTargetView>(rtv),
			                                     ut::ComPtr<ID3D11DepthStencilView>(dsv) });
		}
	}

	// success
	return Target(PlatformRenderTarget(ut::Move(slice_target_views)),
	              image.Move(),
	              info);
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
	Image::Info backbuffer_info;
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
		return ut::MakeError(ut::error::fail, ut::Print(result) + " CreateSwapChain() failed.");
	}

	// extract texture and rtv from the swapchain to be able to construct ve::render::Target object.
	ID3D11Texture2D* backbuffer;
	ID3D11RenderTargetView* rtv;
	ut::Optional<ut::Error> extract_error = ExtractBackBufferTextureAndView(swapchain, backbuffer, rtv);
	if (extract_error)
	{
		return ut::MakeError(extract_error.Move());
	}
	ut::Array<PlatformRenderTarget::SliceRTV> slice_target_views(1);
	slice_target_views.GetFirst().mips.Add({ ut::ComPtr<ID3D11RenderTargetView>(rtv),
	                                         ut::ComPtr<ID3D11DepthStencilView>(nullptr) });

	// initialize render target info
	Target::Info target_info;
	target_info.format = backbuffer_info.format;
	target_info.usage = Target::Info::usage_present;

	// create render target that will be associated with provided viewport
	Image texture(PlatformImage(backbuffer, nullptr), ut::Move(backbuffer_info));
	Target target(PlatformRenderTarget(ut::Move(slice_target_views)), ut::Move(texture), target_info);

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
	ID3D11CommandList* cmd_list = nullptr;
	ID3D11DeviceContext* deferred_context = nullptr;
	if (cmd_buffer_info.level == CmdBuffer::level_secondary)
	{
		HRESULT result = d3d11_device->CreateDeferredContext(0, &deferred_context);
		if (FAILED(result))
		{
			return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create deferred context.");
		}
	}

	return CmdBuffer(PlatformCmdBuffer(cmd_list, deferred_context), cmd_buffer_info);
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

// Creates a framebuffer. All targets must have the same width and height.
//    @param render_pass - const reference to the renderpass to be bound to.
//    @param color_targets - array of color attachments to be bound
//                           to the render pass.
//    @param depth_stencil_target - optional depth-stencil attachment to be
//                                  bound to a render pass.
//    @return - new framebuffer or error if failed.
ut::Result<Framebuffer, ut::Error> Device::CreateFramebuffer(const RenderPass& render_pass,
	                                                         ut::Array<Framebuffer::Attachment> color_attachments,
	                                                         ut::Optional<Framebuffer::Attachment> ds_attachment)
{
	// determine width and heights of the framebuffer in pixels
	ut::uint32 width;
	ut::uint32 height;
	if (color_attachments.GetNum() != 0)
	{
		const Framebuffer::Attachment& attachment = color_attachments.GetFirst();
		const Image& color_img = attachment.target->image;
		width = ut::Max<ut::uint32>(color_img.GetInfo().width >> attachment.mip, 1);
		height = ut::Max<ut::uint32>(color_img.GetInfo().height >> attachment.mip, 1);
	}
	else if (ds_attachment)
	{
		const Framebuffer::Attachment& attachment = ds_attachment.Get();
		const Image& ds_img = attachment.target->image;
		width = ut::Max<ut::uint32>(ds_img.GetInfo().width >> attachment.mip, 1);
		height = ut::Max<ut::uint32>(ds_img.GetInfo().height >> attachment.mip, 1);
	}
	else
	{
		return ut::MakeError(ut::error::invalid_arg, "DirectX 11: no targets for the framebuffer.");
	}

	// initialize info
	Framebuffer::Info info;
	info.width = width;
	info.height = height;

	// success
	return Framebuffer(PlatformFramebuffer(),
	                   info,
	                   ut::Move(color_attachments),
	                   ut::Move(ds_attachment));
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

	// allignment for uniform buffers
	if (info.type == Buffer::uniform && (buffer_desc.ByteWidth % 16 != 0))
	{
		buffer_desc.ByteWidth += (16 - buffer_desc.ByteWidth % 16);
	}

	// bind flags
	switch (info.type)
	{
	case Buffer::vertex:  buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   break;
	case Buffer::index:   buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;    break;
	case Buffer::uniform: buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break;
	case Buffer::storage: buffer_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE; break;
	}

	// cpu access
	if (info.usage == render::memory::gpu_read_cpu_write)
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
		return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 buffer.");
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
			return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 buffer UAV.");
		}

		result = d3d11_device->CreateShaderResourceView(buffer, &desc_srv, &srv);
		if (FAILED(result))
		{
			return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 buffer srv.");
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
				return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 vertex shader.");
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
				return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 hull shader.");
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
				return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 domain shader.");
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
				return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 geometry shader.");
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
				return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 pixel shader.");
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
				return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 compute shader.");
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

	// descriptors of dx11 input layout per-vertex elements
	const UINT vertex_element_count = static_cast<UINT>(info.input_assembly_state.elements.GetNum());
	const UINT instance_element_count = static_cast<UINT>(info.input_assembly_state.instance_elements.GetNum());
	UINT input_element_count = vertex_element_count + instance_element_count;
	ut::Array<D3D11_INPUT_ELEMENT_DESC> input_el_desc(input_element_count);
	for (UINT i = 0; i < vertex_element_count; i++)
	{
		VertexElement& element = info.input_assembly_state.elements[i];

		D3D11_INPUT_ELEMENT_DESC& desc = input_el_desc[i];
		desc.SemanticName = element.semantic_name;
		desc.SemanticIndex = 0;
		desc.Format = ConvertPixelFormatToDX11(element.format);
		desc.InputSlot = 0;
		desc.AlignedByteOffset = element.offset;
		desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc.InstanceDataStepRate = 0;
	}

	// per-instance elements
	for (UINT i = vertex_element_count; i < input_element_count; i++)
	{
		VertexElement& element = info.input_assembly_state.instance_elements[i - vertex_element_count];

		D3D11_INPUT_ELEMENT_DESC& desc = input_el_desc[i];
		desc.SemanticName = element.semantic_name;
		desc.SemanticIndex = 0;
		desc.Format = ConvertPixelFormatToDX11(element.format);
		desc.InputSlot = 1;
		desc.AlignedByteOffset = element.offset;
		desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		desc.InstanceDataStepRate = 1;
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
			return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 input layout.");
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
		return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 rasterizer state.");
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
		return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 depth-stencil state.");
	}

	// check blendstate attachment count
	const size_t blend_attachment_count = info.blend_state.attachments.GetNum();
	if (blend_attachment_count > 8)
	{
		return ut::MakeError(ut::error::fail, "Too many blendstate attachments (max 8 for DX11).");
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
		return ut::MakeError(ut::error::fail, ut::Print(result) + " failed to create d3d11 blend state.");
	}

	// success
	PlatformPipelineState platform_pipeline(input_layout,
	                                        rasterizer_state,
	                                        depthstencil_state,
	                                        blend_state);
	return PipelineState(ut::Move(platform_pipeline), ut::Move(info));
}

// Resets given command buffer. Don't use it to manually reset a primary buffer
// that was already submited to the gpu.
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
	const bool is_secondary_buffer = cmd_buffer.info.level == CmdBuffer::level_secondary;
	ID3D11DeviceContext* dx11_context_ptr = is_secondary_buffer ? cmd_buffer.deferred_context.Get() :
	                                                              immediate_context.Get();
	Context context(PlatformContext(dx11_context_ptr, is_secondary_buffer));
	if (is_secondary_buffer)
	{
		UT_ASSERT(render_pass);
		UT_ASSERT(framebuffer);

		// all render targets must be set separately for the deferred context,
		// clear values will be ignored
		ut::Rect<ut::uint32> render_area(0, 0,
		                                 framebuffer->info.width,
		                                 framebuffer->info.height);
		context.BeginRenderPass(render_pass.Get(),
		                        framebuffer.Get(),
		                        render_area,
		                        ut::Color<4>(0),
		                        1.0f, 0, false);
	}

	function(ut::Move(context));

	if (is_secondary_buffer)
	{
		ID3D11CommandList* cmd_list;
		cmd_buffer.deferred_context->FinishCommandList(0, &cmd_list);
		cmd_buffer.cmd_list = ut::ComPtr<ID3D11CommandList>(cmd_list);
	}
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