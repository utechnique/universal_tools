//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_platform.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts pixel format to the one compatible with Vulkan.
ut::Array<const char*> GetPlatformVkInstanceExtensions()
{
	ut::Array<const char*> extensions;
	extensions.Add(VK_KHR_SURFACE_EXTENSION_NAME);
#if UT_WINDOWS
	extensions.Add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif UT_LINUX
	extensions.Add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#else
#error GetPlatformVkExtensions() is not implemented.
#endif
	return extensions;
}

// Vulkan device extensions specific to the current platform.
ut::Array<const char*> GetDeviceVkInstanceExtensions()
{
	ut::Array<const char*> extensions;
	extensions.Add(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	return extensions;
}

// Creates ut::Error from the VkResult value
ut::Error VulkanError(VkResult res, const ut::String& desc)
{
	return ut::Error(ut::error::fail, desc + " " + ut::Print<ut::uint32>(static_cast<ut::uint32>(res)));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//