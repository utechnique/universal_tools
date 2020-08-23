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
	case pixel::r8g8b8: return VK_FORMAT_R8G8B8_UNORM;
	case pixel::b8g8r8: return VK_FORMAT_B8G8R8_UNORM;
	case pixel::r8g8b8a8: return VK_FORMAT_R8G8B8A8_UNORM;
	case pixel::b8g8r8a8: return VK_FORMAT_B8G8R8A8_UNORM;
	case pixel::r8g8b8a8_srgb: return VK_FORMAT_R8G8B8A8_SRGB;
	case pixel::b8g8r8a8_srgb: return VK_FORMAT_B8G8R8A8_SRGB;
	case pixel::r32: return VK_FORMAT_R32_SFLOAT;
	case pixel::r32g32: return VK_FORMAT_R32G32_SFLOAT;
	case pixel::r32g32b32: return VK_FORMAT_R32G32B32_SFLOAT;
	case pixel::r32g32b32a32: return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	return VK_FORMAT_UNDEFINED;
}

// Converts Vulkan pixel format to ve::render::pixel::Format value.
pixel::Format ConvertPixelFormatFromVulkan(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8G8B8_UNORM: return pixel::r8g8b8;
	case VK_FORMAT_B8G8R8_UNORM: return pixel::b8g8r8;
	case VK_FORMAT_R8G8B8A8_UNORM: return pixel::r8g8b8a8;
	case VK_FORMAT_B8G8R8A8_UNORM: return pixel::b8g8r8a8;
	case VK_FORMAT_R8G8B8A8_SRGB: return pixel::r8g8b8a8_srgb;
	case VK_FORMAT_B8G8R8A8_SRGB: return pixel::b8g8r8a8_srgb;
	case VK_FORMAT_R32_SFLOAT: return pixel::r32;
	case VK_FORMAT_R32G32_SFLOAT: return pixel::r32g32;
	case VK_FORMAT_R32G32B32_SFLOAT: return pixel::r32g32b32;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return pixel::r32g32b32a32;
	}
	return pixel::unknown;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//