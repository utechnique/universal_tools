//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_pixel_format.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts pixel format to the one compatible with Vulkan.
VkFormat ConvertPixelFormatToVulkan(pixel::Format format)
{
	switch (format)
	{
	case pixel::Format::r8g8b8a8_unorm: return VK_FORMAT_R8G8B8A8_UNORM;
	case pixel::Format::b8g8r8a8_unorm: return VK_FORMAT_B8G8R8A8_UNORM;
	case pixel::Format::r8g8b8a8_srgb: return VK_FORMAT_R8G8B8A8_SRGB;
	case pixel::Format::b8g8r8a8_srgb: return VK_FORMAT_B8G8R8A8_SRGB;
	case pixel::Format::a2b10g10r10_unorm: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case pixel::Format::r11g11b10_float: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	case pixel::Format::r16_float: return VK_FORMAT_R16_SFLOAT;
	case pixel::Format::r16_sint: return VK_FORMAT_R16_SINT;
	case pixel::Format::r16_uint: return VK_FORMAT_R16_UINT;
	case pixel::Format::r16_snorm: return VK_FORMAT_R16_SNORM;
	case pixel::Format::r16_unorm: return VK_FORMAT_R16_UNORM;
	case pixel::Format::r16g16_float: return VK_FORMAT_R16G16_SFLOAT;
	case pixel::Format::r16g16_sint: return VK_FORMAT_R16G16_SINT;
	case pixel::Format::r16g16_uint: return VK_FORMAT_R16G16_UINT;
	case pixel::Format::r16g16_snorm: return VK_FORMAT_R16G16_SNORM;
	case pixel::Format::r16g16_unorm: return VK_FORMAT_R16G16_UNORM;
	case pixel::Format::r16g16b16a16_float: return VK_FORMAT_R16G16B16A16_SFLOAT;
	case pixel::Format::r16g16b16a16_sint: return VK_FORMAT_R16G16B16A16_SINT;
	case pixel::Format::r16g16b16a16_uint: return VK_FORMAT_R16G16B16A16_UINT;
	case pixel::Format::r16g16b16a16_snorm: return VK_FORMAT_R16G16B16A16_SNORM;
	case pixel::Format::r16g16b16a16_unorm: return VK_FORMAT_R16G16B16A16_UNORM;
	case pixel::Format::r32_float: return VK_FORMAT_R32_SFLOAT;
	case pixel::Format::r32_sint: return VK_FORMAT_R32_SINT;
	case pixel::Format::r32_uint: return VK_FORMAT_R32_UINT;
	case pixel::Format::r32g32_float: return VK_FORMAT_R32G32_SFLOAT;
	case pixel::Format::r32g32_sint: return VK_FORMAT_R32G32_SINT;
	case pixel::Format::r32g32_uint: return VK_FORMAT_R32G32_UINT;
	case pixel::Format::r32g32b32_float: return VK_FORMAT_R32G32B32_SFLOAT;
	case pixel::Format::r32g32b32_sint: return VK_FORMAT_R32G32B32_SINT;
	case pixel::Format::r32g32b32_uint: return VK_FORMAT_R32G32B32_UINT;
	case pixel::Format::r32g32b32a32_float: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case pixel::Format::r32g32b32a32_sint: return VK_FORMAT_R32G32B32A32_SINT;
	case pixel::Format::r32g32b32a32_uint: return VK_FORMAT_R32G32B32A32_UINT;
	case pixel::Format::d16_unorm: return VK_FORMAT_D16_UNORM;
	case pixel::Format::d24_unorm_s8_uint: return VK_FORMAT_D24_UNORM_S8_UINT;
	case pixel::Format::d32_float: return VK_FORMAT_D32_SFLOAT;
	case pixel::Format::d32_float_s8_uint: return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
	return VK_FORMAT_UNDEFINED;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//