//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "ut.h"
#include "vulkan/vulkan.h"
//----------------------------------------------------------------------------//
#if UT_WINDOWS
#include "vulkan/vulkan_win32.h"
#elif UT_LINUX
#include "X11/Xlib.h"
#include "vulkan/vulkan_xlib.h"
#endif
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan instance extensions specific to the current platform.
ut::Array<const char*> GetPlatformVkInstanceExtensions();

// Vulkan device extensions specific to the current platform.
ut::Array<const char*> GetDeviceVkInstanceExtensions();

// Creates ut::Error from the VkResult value
ut::Error VulkanError(VkResult res, const ut::String& desc = ut::String());

// Positive skEnableVulkanValidationLayer value enables vulkan validation layer.
#if DEBUG
static const bool skEnableVulkanValidationLayer = true;
#else
static const bool skEnableVulkanValidationLayer = false;
#endif

// Validation layer name.
static const char* skVulkanValidationLayerName = "VK_LAYER_KHRONOS_validation";


//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
