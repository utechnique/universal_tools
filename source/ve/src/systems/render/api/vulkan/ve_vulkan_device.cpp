//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_device.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Maximum number of swapchains to present simultaneously.
const ut::uint32 skMaxVkSwapchainPresent = 32;

// Infinite vulkan fence idle.
const uint64_t skFenceInfiniteTimeout = 100000000;

//----------------------------------------------------------------------------//
// Constructor.
PlatformDevice::PlatformDevice() : instance(CreateVulkanInstance())
                                 , gpu(nullptr)
{
	// create dbg messenger
	dbg_messenger = VkRc<vk::dbg_messenger>(CreateDbgMessenger(), instance.GetVkHandle());

	// create device
	device = VkRc<vk::device>(CreateVulkanDevice());

	// create queues
	main_queue = CreateQueue(vulkan_queue::main, 0);

	// command pools
	dynamic_cmd_pool = CreateCmdPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	static_cmd_pool = CreateCmdPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

// Destructor.
PlatformDevice::~PlatformDevice()
{
	vkDeviceWaitIdle(device.GetVkHandle());
}

// Move constructor.
PlatformDevice::PlatformDevice(PlatformDevice&&) noexcept = default;

// Move operator.
PlatformDevice& PlatformDevice::operator =(PlatformDevice&&) noexcept = default;

// Creates vkInstance object.
VkInstance PlatformDevice::CreateVulkanInstance()
{
	// name of the application/engine
	const char* app_name = "virtual_environment";

	// initialize the VkApplicationInfo structure
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = app_name;
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);;
	app_info.pEngineName = app_name;
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);;
	app_info.apiVersion = VK_API_VERSION_1_0;

	// instance information object
	VkInstanceCreateInfo inst_info = {};

	// get vulkan extensions specific to the current platform
	ut::Array<const char*> extensions = GetPlatformVkInstanceExtensions();

	// possible layer wrappers
	ut::Array<const char*> layers;

	// initialize validation layer
	if (skEnableVulkanValidationLayer)
	{
		if (CheckValidationLayerSupport())
		{
			layers.Add(skVulkanValidationLayerName);
			extensions.Add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			ut::log << "Vulkan: validation layer enabled." << ut::cret;
		}
		else
		{
			ut::log << "Vulkan: validation layer is not supported." << ut::cret;
		}
	}

	// initialize the VkInstanceCreateInfo structure
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = static_cast<uint32_t>(extensions.GetNum());
	inst_info.ppEnabledExtensionNames = extensions.GetAddress();
	inst_info.enabledLayerCount = static_cast<uint32_t>(layers.GetNum());
	inst_info.ppEnabledLayerNames = layers.GetAddress();

	// create instance itself
	VkInstance out_instance;
	VkResult res = vkCreateInstance(&inst_info, nullptr, &out_instance);
	if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		throw ut::Error(ut::error::not_supported, "Cannot find a compatible Vulkan ICD.");
	}
	else if (res == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan extension is not supported.");
	}
	else if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateInstance");
	}

	// success
	return out_instance;
}

// Returns 'true' if system supports validation layer.
bool PlatformDevice::CheckValidationLayerSupport()
{
	// get number of layers
	uint32_t layer_count;
	VkResult res = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumerateInstanceLayerProperties(count)");
	}

	// get layer properties
	ut::Array<VkLayerProperties> available_layers(layer_count);
	res = vkEnumerateInstanceLayerProperties(&layer_count, available_layers.GetAddress());
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumerateInstanceLayerProperties(properties)");
	}

	// check if our validation layer name matches any
	for (size_t i = 0; i < layer_count; i++)
	{
		if (ut::StrCmp<char>(skVulkanValidationLayerName, available_layers[i].layerName))
		{
			return true;
		}
	}

	// not found
	return false;
}

// Returns physical device that suits best.
ut::Optional<VkPhysicalDevice> PlatformDevice::SelectPreferredPhysicalDevice(const ut::Array<VkPhysicalDevice>& devices)
{
	if (devices.GetNum() == 0)
	{
		throw ut::Error(ut::error::empty, "Gpu list is empty");
	}

	ut::Optional< ut::Pair<VkPhysicalDevice, ut::uint32> > top_scored_device;

	for (size_t i = 0; i < devices.GetNum(); i++)
	{
		ut::uint32 score = 0;

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(devices[i], &properties);
		vkGetPhysicalDeviceFeatures(devices[i], &features);

		// geometry and tessellation shaders are necessary
		if (!features.geometryShader || !features.tessellationShader)
		{
			continue;
		}

		// it's better to use discrete gpu
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1;
		}

		if (!top_scored_device || score > top_scored_device->second)
		{
			top_scored_device = ut::Pair<VkPhysicalDevice, ut::uint32>(devices[i], score);
		}		
	}

	if (top_scored_device)
	{
		return top_scored_device->first;
	}

	return ut::Optional<VkPhysicalDevice>();
}

// Vulkan message callback.
VKAPI_ATTR VkBool32 VKAPI_CALL PlatformDevice::VulkanDbgCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT type,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                                 void* user_data)
{
	ut::log << "Vulkan: " << callback_data->pMessage << ut::cret;
	if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		throw ut::Error(ut::error::fail, callback_data->pMessage);
	}
	return VK_FALSE;
}

// Returns an array of physical devices.
ut::Array<VkPhysicalDevice> PlatformDevice::EnumeratePhysicalDevices()
{
	// get number of physical devices
	uint32_t gpu_count = 1;
	VkResult res = vkEnumeratePhysicalDevices(instance.GetVkHandle(), &gpu_count, nullptr);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumeratePhysicalDevices(count)");
	}
	else if (gpu_count == 0)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan can't detect any gpu adapter.");
	}

	// enumerate physical devices
	ut::Array<VkPhysicalDevice> physical_devices(gpu_count);
	res = vkEnumeratePhysicalDevices(instance.GetVkHandle(), &gpu_count, physical_devices.GetAddress());
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkEnumeratePhysicalDevices");
	}
	else if (gpu_count == 0)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan can't enumerate physical devices.");
	}

	return physical_devices;
}

// Returns an array of queue family properties.
ut::Map<vulkan_queue::FamilyType, vulkan_queue::Family> PlatformDevice::GetQueueFamilies(VkPhysicalDevice physical_device)
{
	// get number of queue families
	uint32_t queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
	if (queue_family_count == 0)
	{
		throw ut::Error(ut::error::fail, "vkGetPhysicalDeviceQueueFamilyProperties returned zero.");
	}

	// get all families
	ut::Array<VkQueueFamilyProperties> properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, properties.GetAddress());
	if (queue_family_count == 0)
	{
		throw ut::Error(ut::error::fail, "vkGetPhysicalDeviceQueueFamilyProperties returned empty list of properties.");
	}

	// select needed
	ut::Map<vulkan_queue::FamilyType, vulkan_queue::Family> out_map;
	for (uint32_t i = 0; i < queue_family_count; i++)
	{
		if (properties[i].queueCount == 0)
		{
			continue;
		}

		vulkan_queue::Family family;
		family.id = i;
		family.count = properties[i].queueCount;

		// main
		if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT &&
		    properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
		    properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			out_map.Insert(vulkan_queue::main, ut::Move(family));
		} // compute
		else if (properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
		         !(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		         !(properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
		{
			out_map.Insert(vulkan_queue::compute, ut::Move(family));
		} // transfer
		else if (properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
		         !(properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		         !(properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			out_map.Insert(vulkan_queue::compute, ut::Move(family));
		}
	}

	// success
	return out_map;
}

// Create intercepter for messages generated by Vulkan instance.
VkDebugUtilsMessengerEXT PlatformDevice::CreateDbgMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT messenger_info{};
	messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messenger_info.pfnUserCallback = PlatformDevice::VulkanDbgCallback;
	messenger_info.pUserData = nullptr; // Optional
	
	auto create_messenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance.GetVkHandle(), "vkCreateDebugUtilsMessengerEXT");
	if (create_messenger == nullptr)
	{
		throw ut::Error(ut::error::not_supported, "vkCreateDebugUtilsMessengerEXT function is not supported");
	}

	VkDebugUtilsMessengerEXT out_messenger;
	VkResult res = create_messenger(instance.GetVkHandle(), &messenger_info, nullptr, &out_messenger);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateDebugUtilsMessengerEXT");
	}
	
	return out_messenger;
}

// Creates VkDevice object.
VkDevice PlatformDevice::CreateVulkanDevice()
{
	// enumerate physical devices
	ut::Array<VkPhysicalDevice> physical_devices = EnumeratePhysicalDevices();

	// select suitable gpu
	ut::Optional<VkPhysicalDevice> preferred_gpu = SelectPreferredPhysicalDevice(physical_devices);
	if (!preferred_gpu)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan: no suitable GPU.");
	}

	// extract gpu name
	gpu = preferred_gpu.Get();
	VkPhysicalDeviceProperties gpu_properties;
	vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
	ut::log << "Vulkan: using " << gpu_properties.deviceName << " for rendering." << ut::cret;

	// retrieve queue family properties
	queue_families = PlatformDevice::GetQueueFamilies(gpu);

	// find graphics queue id
	ut::Optional<vulkan_queue::Family&> main_queue_family  = queue_families.Find(vulkan_queue::main);

	// validate graphics queue family
	if (!main_queue_family)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan: General purpose queue is not supported.");
	}

	// VkDeviceQueueCreateInfo
	float queue_priorities[1] = { 0.0 };
	VkDeviceQueueCreateInfo queue_info = {};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.pNext = nullptr;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = queue_priorities;
	queue_info.queueFamilyIndex = main_queue_family->id;

	// get device extensions
	ut::Array<const char*> extensions = GetDeviceVkInstanceExtensions();

	// VkDeviceCreateInfo
	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = nullptr;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = static_cast<uint32_t>(extensions.GetNum());
	device_info.ppEnabledExtensionNames = extensions.GetAddress();
	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = nullptr;
	device_info.pEnabledFeatures = nullptr;

	// create device
	VkDevice out_device;
	VkResult res = vkCreateDevice(gpu, &device_info, nullptr, &out_device);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateDevice");
	}

	// success
	return out_device;
}

// Creates desired queue.
VkRc<vk::queue> PlatformDevice::CreateQueue(vulkan_queue::FamilyType family_type, uint32_t queue_id)
{
	ut::Optional<vulkan_queue::Family&> family = queue_families.Find(family_type);
	if (!family)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan: queue type is not supported.");
	}

	const uint32_t family_id = family->id;

	VkQueue queue;
	vkGetDeviceQueue(device.GetVkHandle(), family_id, queue_id, &queue);

	VkDetail<vk::queue> detail(queue_id, family_id);
	return VkRc<vk::queue>(queue, detail);
}

// Creates command pool.
VkRc<vk::cmd_pool> PlatformDevice::CreateCmdPool(VkCommandPoolCreateFlags flags)
{
	ut::Optional<vulkan_queue::Family&> main_queue_family = queue_families.Find(vulkan_queue::main);
	if (!main_queue_family)
	{
		throw ut::Error(ut::error::not_supported, "Vulkan: General purpose queue is not supported.");
	}

	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = nullptr;
	cmd_pool_info.queueFamilyIndex = main_queue_family->id;
	cmd_pool_info.flags = flags;

	VkCommandPool out_cmd_pool;
	VkResult res = vkCreateCommandPool(device.GetVkHandle(), &cmd_pool_info, nullptr, &out_cmd_pool);
	if (res != VK_SUCCESS)
	{
		throw VulkanError(res, "vkCreateCommandPool");
	}

	return VkRc<vk::cmd_pool>(out_cmd_pool, device.GetVkHandle());
}

//----------------------------------------------------------------------------//
// Converts load operation value to vulkan format.
VkAttachmentLoadOp ConvertLoadOpToVulkan(RenderTargetSlot::LoadOperation load_op)
{
	switch (load_op)
	{
		case RenderTargetSlot::load_extract: return VK_ATTACHMENT_LOAD_OP_LOAD;
		case RenderTargetSlot::load_clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case RenderTargetSlot::load_dont_care: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

// Converts store operation value to vulkan format.
VkAttachmentStoreOp ConvertStoreOpToVulkan(RenderTargetSlot::StoreOperation store_op)
{
	switch (store_op)
	{
		case RenderTargetSlot::store_save: return VK_ATTACHMENT_STORE_OP_STORE;
		case RenderTargetSlot::store_dont_care: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

//----------------------------------------------------------------------------//
// Constructor.
Device::Device(ut::SharedPtr<ui::Frontend::Thread> ui_frontend) : PlatformDevice()
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
	PlatformImage platform_texture;
	return Image(ut::Move(platform_texture), info);
}

// Creates platform-specific representation of the rendering area inside a UI viewport.
//    @param viewport - reference to UI viewport containing rendering area.
//    @param vsync - boolean whether to enable vertical synchronization or not.
//    @return - new display object or error if failed.
ut::Result<Display, ut::Error> Device::CreateDisplay(ui::PlatformViewport& viewport, bool vsync)
{
	// surface of the display
	VkSurfaceKHR surface;

#if UT_WINDOWS
	// extract windows handle from the viewport widget
	const HWND hwnd = fl_xid(&viewport);
	HINSTANCE hinstance = fl_display;

	// fill info stuct object before creating surface
	VkWin32SurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.hinstance = fl_display;
	create_info.hwnd = hwnd;

	// create surface
	VkResult res = vkCreateWin32SurfaceKHR(instance.GetVkHandle(), &create_info, nullptr, &surface);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateWin32SurfaceKHR"));
	}
#elif UT_LINUX && VE_X11
	// fill info stuct object before creating surface
	VkXlibSurfaceCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.flags = 0;
	create_info.dpy = fl_display;
	create_info.window = fl_xid(&viewport);

	// create surface
	VkResult res = vkCreateXlibSurfaceKHR(instance.GetVkHandle(), &create_info, nullptr, &surface);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateXlibSurfaceKHR"));
	}
#endif
	
	// check if main queue supports present
	VkBool32 supports_present;
	res = vkGetPhysicalDeviceSurfaceSupportKHR(gpu, main_queue.GetDetail().GetQueueFamilyId(), surface, &supports_present);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceSupportKHR"));
	}

	// if present is not supported - there is no way to display an image to user
	if (!supports_present)
	{
		return ut::MakeError(ut::Error(ut::error::not_supported, "Vulkan: present feauture is not supported."));
	}

	// get the number of VkFormats that are supported by surface
	uint32_t format_count;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, nullptr);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	}

	// get the list of VkFormats that are supported
	ut::Array<VkSurfaceFormatKHR> surf_formats(format_count);
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, surf_formats.GetAddress());
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	}

	// validate format count, it must have at least one value
	if (format_count == 0)
	{
		return ut::MakeError(ut::Error(ut::error::empty, "Vulkan: empty format list for given surface."));
	}

	// if the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format; otherwise, at least one
	// supported format will be returned
	VkFormat surface_format;
	if (format_count == 1 && surf_formats[0].format == VK_FORMAT_UNDEFINED)
	{
		surface_format = VK_FORMAT_R8G8B8A8_SRGB;
	}
	else
	{
		// search if srgb is supported
		bool found_srgb = false;
		for (uint32_t i = 0; i < format_count; i++)
		{
			const VkFormat supported_format = surf_formats[i].format;
			if (supported_format == VK_FORMAT_R8G8B8_SRGB ||
				supported_format == VK_FORMAT_B8G8R8_SRGB ||
				supported_format == VK_FORMAT_R8G8B8A8_SRGB ||
				supported_format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				surface_format = supported_format;
				found_srgb = true;
				break;
			}
		}

		// otherwise use first supported
		if (!found_srgb)
		{
			surface_format = surf_formats[0].format;
		}
	}
	
	// surface capabilities
	VkSurfaceCapabilitiesKHR surf_capabilities;
	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surf_capabilities);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	}

	// get the number of present modes
	uint32_t present_mode_count;
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, nullptr);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfacePresentModesKHR(count)"));
	}

	// get the list of present modes
	ut::Array<VkPresentModeKHR> present_modes(present_mode_count);
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, present_modes.GetAddress());
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
	}
	
	// get size of the viewport
	const ut::uint32 width = static_cast<ut::uint32>(viewport.w());
	const ut::uint32 height = static_cast<ut::uint32>(viewport.h());

	// width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
	VkExtent2D swapchain_extent;
	if (surf_capabilities.currentExtent.width == 0xFFFFFFFF)
	{
		swapchain_extent.width = ut::Clamp(width,
		                                   surf_capabilities.minImageExtent.width,
		                                   surf_capabilities.maxImageExtent.width);
		swapchain_extent.height = ut::Clamp(height,
		                                    surf_capabilities.minImageExtent.height,
		                                    surf_capabilities.maxImageExtent.height);
	}
	else
	{
		// if the surface size is defined, the swap chain size must match
		swapchain_extent = surf_capabilities.currentExtent;
	}

	// the FIFO present mode is guaranteed by the spec to be supported
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

	// check if immediate mode is supported
	if (!vsync)
	{
		// switch back to vsync enabled state until it's known
		// that immediate mode is supported
		vsync = true;

		// iterate all modes to find immediate one
		for (uint32_t i = 0; i < present_mode_count; i++)
		{
			if (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				vsync = false;
			}
		}
	}

	// Determine the number of VkImage's to use in the swap chain.
	// We need to acquire only 1 presentable image at at time.
	// Asking for minImageCount images ensures that we can acquire
	// 1 presentable image as long as we present it before attempting
	// to acquire another.
	uint32_t desired_number_of_swap_chain_images = surf_capabilities.minImageCount;

	// surface transform
	VkSurfaceTransformFlagBitsKHR pre_transform;
	if (surf_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		pre_transform = surf_capabilities.currentTransform;
	}

	// Find a supported composite alpha mode - one of these is guaranteed to be set
	VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] =
	{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (uint32_t i = 0; i < sizeof(composite_alpha_flags) / sizeof(composite_alpha_flags[0]); i++)
	{
		if (surf_capabilities.supportedCompositeAlpha & composite_alpha_flags[i])
		{
			composite_alpha = composite_alpha_flags[i];
			break;
		}
	}

	// fill swapchain info object
	VkSwapchainCreateInfoKHR swapchain_ci = {};
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_ci.pNext = nullptr;
	swapchain_ci.surface = surface;
	swapchain_ci.minImageCount = desired_number_of_swap_chain_images;
	swapchain_ci.imageFormat = surface_format;
	swapchain_ci.imageExtent.width = swapchain_extent.width;
	swapchain_ci.imageExtent.height = swapchain_extent.height;
	swapchain_ci.preTransform = pre_transform;
	swapchain_ci.compositeAlpha = composite_alpha;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = present_mode;
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
	swapchain_ci.clipped = true;
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = nullptr;

	// create swapchain for this viewport
	VkSwapchainKHR swap_chain;
	res = vkCreateSwapchainKHR(device.GetVkHandle(), &swapchain_ci, nullptr, &swap_chain);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateSwapchainKHR"));
	}

	// get buffer count
	uint32_t image_count;
	res = vkGetSwapchainImagesKHR(device.GetVkHandle(), swap_chain, &image_count, nullptr);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetSwapchainImagesKHR(count)"));
	}

	// get back and front buffers
	ut::Array<VkImage> swapchain_images(image_count);
	res = vkGetSwapchainImagesKHR(device.GetVkHandle(), swap_chain, &image_count, swapchain_images.GetAddress());
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkGetSwapchainImagesKHR"));
	}

	// create render targets for all buffers in a swapchain
	ut::Array<Target> targets;
	for (uint32_t i = 0; i < image_count; i++)
	{
		// initialize image view info
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = nullptr;
		color_image_view.flags = 0;
		color_image_view.image = swapchain_images[i];
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.format = surface_format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;

		// create image view for the current buffer
		VkImageView view;
		res = vkCreateImageView(device.GetVkHandle(), &color_image_view, nullptr, &view);
		if (res != VK_SUCCESS)
		{
			return ut::MakeError(VulkanError(res, "vkCreateImageView(swapchain)"));
		}

		// platform render target
		PlatformRenderTarget platform_target(device.GetVkHandle(), view);

		// stub texture
		ImageInfo info;
		info.format = ConvertPixelFormatFromVulkan(surface_format);
		info.width = swapchain_extent.width;
		info.height = swapchain_extent.height;
		info.depth = 1;
		Image image(PlatformImage(), info);

		// initialize render target info
		RenderTargetInfo target_info;
		target_info.usage = RenderTargetInfo::usage_present;

		// create final target for the current buffer
		Target target(ut::Move(platform_target), ut::Move(image), target_info);
		if (!targets.Add(ut::Move(target)))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// create platform-specific display object
	PlatformDisplay platform_display(instance.GetVkHandle(), device.GetVkHandle(), surface, swap_chain, image_count);

	// create final display object
	Display out_display(ut::Move(platform_display), ut::Move(targets), width, height, vsync);

	// retreive first buffer id to be filled
	out_display.current_buffer_id = out_display.AcquireNextBuffer();

	// success
	return out_display;
}

// Creates an empty command buffer.
//    @param cmd_buffer_info - reference to the information about
//                             the command buffer to be created.
ut::Result<CmdBuffer, ut::Error> Device::CreateCmdBuffer(const CmdBufferInfo& cmd_buffer_info)
{
	bool is_dynamic = (cmd_buffer_info.usage & CmdBufferInfo::usage_once) ? true : false;
	bool is_primary = cmd_buffer_info.level == CmdBufferInfo::level_primary;

	VkCommandBufferAllocateInfo cmd_info = {};
	cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_info.pNext = nullptr;
	cmd_info.commandPool = is_dynamic ? dynamic_cmd_pool.GetVkHandle() : static_cmd_pool.GetVkHandle();
	cmd_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	cmd_info.commandBufferCount = 1;

	VkCommandBuffer cmd_buffer;
	VkResult res = vkAllocateCommandBuffers(device.GetVkHandle(), &cmd_info, &cmd_buffer);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkAllocateCommandBuffers"));
	}

	VkFenceCreateInfo fence_info;
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.pNext = nullptr;
	fence_info.flags = 0;

	VkFence fence;
	res = vkCreateFence(device.GetVkHandle(), &fence_info, nullptr, &fence);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateFence(cmdbuffer)"));
	}

	PlatformCmdBuffer platform_buffer(device.GetVkHandle(), dynamic_cmd_pool.GetVkHandle(), cmd_buffer, fence);
	return CmdBuffer(ut::Move(platform_buffer), cmd_buffer_info);
}

// Creates render pass object.
//    @param in_color_slots - array of color slots.
//    @param in_depth_stencil_slot - optional depth stencil slot.
//    @return - new render pass or error if failed.
ut::Result<RenderPass, ut::Error> Device::CreateRenderPass(ut::Array<RenderTargetSlot> in_color_slots,
	                                                       ut::Optional<RenderTargetSlot> in_depth_stencil_slot)
{
	ut::Array<VkAttachmentDescription> attachments;

	// initialize color attachments
	bool has_present_surface = false;
	ut::Array<VkAttachmentReference> color_references;
	for (size_t i = 0; i < in_color_slots.GetNum(); i++)
	{
		VkAttachmentDescription color_attachment;

		// convert pixel format
		color_attachment.format = ConvertPixelFormatToVulkan(in_color_slots[i].format);

		// always 1 sample
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		// load/store operations
		color_attachment.loadOp = ConvertLoadOpToVulkan(in_color_slots[i].load_op);
		color_attachment.storeOp = ConvertStoreOpToVulkan(in_color_slots[i].store_op);

		// no stencil operations
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// initial layout
		if (in_color_slots[i].load_op == RenderTargetSlot::load_clear)
		{
			color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		// final layout
		if (in_color_slots[i].present_surface)
		{
			color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			has_present_surface = true;
		}
		else
		{
			color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		// no flags
		color_attachment.flags = 0;

		// add attachment to the array
		if (!attachments.Add(color_attachment))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}

		// create reference
		VkAttachmentReference color_reference = {};
		color_reference.attachment = static_cast<uint32_t>(i);
		color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// add reference to the array
		if (!color_references.Add(color_reference))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// initialize depth-stencil attachment
	VkAttachmentReference depth_reference = {};
	if (in_depth_stencil_slot)
	{
		RenderTargetSlot& depth_stencil_slot = in_depth_stencil_slot.Get();

		VkAttachmentDescription depth_attachment;

		// convert pixel format
		depth_attachment.format = ConvertPixelFormatToVulkan(depth_stencil_slot.format);

		// always 1 sample
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		// load/store operations
		depth_attachment.loadOp = ConvertLoadOpToVulkan(depth_stencil_slot.load_op);
		depth_attachment.storeOp = ConvertStoreOpToVulkan(depth_stencil_slot.store_op);
		depth_attachment.stencilLoadOp = ConvertLoadOpToVulkan(depth_stencil_slot.load_op);
		depth_attachment.stencilStoreOp = ConvertStoreOpToVulkan(depth_stencil_slot.store_op);

		// initial layout
		if (depth_stencil_slot.load_op == RenderTargetSlot::load_clear)
		{
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		// final layout
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// no flags
		depth_attachment.flags = 0;

		// add to the array
		if (!attachments.Add(depth_attachment))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}

		// initialize reference
		depth_reference.attachment = 1;
		depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = static_cast<uint32_t>(color_references.GetNum());
	subpass.pColorAttachments = color_references.GetAddress();
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = in_depth_stencil_slot ? &depth_reference : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	// subpass dependency to wait for wsi image acquired
	// semaphore before starting layout transition
	VkSubpassDependency subpass_dependency = {};
	subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.srcAccessMask = 0;
	subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dependency.dependencyFlags = 0;

	// initialize render pass info
	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = nullptr;
	rp_info.attachmentCount = static_cast<uint32_t>(attachments.GetNum());
	rp_info.pAttachments = attachments.GetAddress();
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = has_present_surface ? 1 : 0;
	rp_info.pDependencies = has_present_surface ? &subpass_dependency : nullptr;

	// create render pass
	VkRenderPass render_pass;
	VkResult res = vkCreateRenderPass(device.GetVkHandle(), &rp_info, nullptr, &render_pass);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateRenderPass"));
	}

	// success
	PlatformRenderPass platform_render_pass(device.GetVkHandle(), render_pass);
	return RenderPass(ut::Move(platform_render_pass), ut::Move(in_color_slots), ut::Move(in_depth_stencil_slot));
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
		return ut::MakeError(ut::Error(ut::error::invalid_arg, "Vulkan: no targets for framebuffer."));
	}

	// color attachments
	ut::Array<VkImageView> attachments;
	const size_t color_target_count = color_targets.GetNum();
	for (size_t i = 0; i < color_target_count; i++)
	{
		const Target& color_target = color_targets[i];

		// check width and height
		const ImageInfo& img_info = color_target.image.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "Vulkan: different width/height for framebuffer."));
		}

		// add attachment
		if (!attachments.Add(color_target.image_view.GetVkHandle()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// depth attachment
	if (depth_stencil_target)
	{
		const Target& ds_target = depth_stencil_target.Get();

		// check width and height
		const ImageInfo& img_info = ds_target.image.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "Vulkan: different width/height for framebuffer."));
		}

		// add attachment
		if (!attachments.Add(ds_target.image_view.GetVkHandle()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// framebuffer info
	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext = nullptr;
	fb_info.renderPass = render_pass.render_pass.GetVkHandle();
	fb_info.attachmentCount = static_cast<uint32_t>(attachments.GetNum());
	fb_info.pAttachments = attachments.GetAddress();
	fb_info.width = static_cast<uint32_t>(width);
	fb_info.height = static_cast<uint32_t>(height);
	fb_info.layers = 1;

	// create framebuffer
	VkFramebuffer framebuffer;
	VkResult res = vkCreateFramebuffer(device.GetVkHandle(), &fb_info, nullptr, &framebuffer);
	if (res != VK_SUCCESS)
	{
		return ut::MakeError(VulkanError(res, "vkCreateFramebuffer"));
	}

	// success
	PlatformFramebuffer platform_framebuffer(device.GetVkHandle(), framebuffer);
	return Framebuffer(ut::Move(platform_framebuffer),
	                   FramebufferInfo(width, height),
	                   ut::Move(color_targets),
	                   ut::Move(depth_stencil_target));
}

// Resets all command buffers created with CmdBufferInfo::usage_once flag
// enabled. This call is required before re-recording such command buffers.
void Device::ResetDynamicCmdPool()
{
	VkResult res = VK_SUCCESS;

	// wait for all dynamic buffers
	const uint32_t fence_count = static_cast<uint32_t>(dynamic_cmd_fences.GetNum());
	if (fence_count != 0)
	{
		do {
			res = vkWaitForFences(device.GetVkHandle(),
			                      fence_count,
			                      dynamic_cmd_fences.GetAddress(),
			                      VK_TRUE,
			                      skFenceInfiniteTimeout);
		} while (res == VK_TIMEOUT);
		UT_ASSERT(res == VK_SUCCESS);

		// reset fences
		vkResetFences(device.GetVkHandle(), fence_count, dynamic_cmd_fences.GetAddress());

		// clear fence array
		dynamic_cmd_fences.Empty();
	}

	// reset pool
	res = vkResetCommandPool(device.GetVkHandle(), dynamic_cmd_pool.GetVkHandle(), 0);
	UT_ASSERT(res == VK_SUCCESS);

	// get next buffer for all displays
	const size_t display_count = swap_buffer_queue.GetNum();
	for (size_t i = 0; i < display_count; i++)
	{
		Display& display = static_cast<Display&>(swap_buffer_queue[i].Get());
		display.current_buffer_id = display.AcquireNextBuffer();
	}
	swap_buffer_queue.Empty();
}

// Resets given command buffer. This command buffer must be created without
// CmdBufferInfo::usage_once flag (use ResetCmdPool() instead).
//    @param cmd_buffer - reference to the buffer to be reset.
void Device::ResetCmdBuffer(CmdBuffer& cmd_buffer)
{
	if (cmd_buffer.info.usage & CmdBufferInfo::usage_once)
	{
		throw ut::Error(ut::error::invalid_arg, "Render: provided command buffer can't be reset.");
	}
	VkResult res = vkResetCommandBuffer(cmd_buffer.buffer.GetVkHandle(), 0);
	UT_ASSERT(res == VK_SUCCESS);
}

// Records commands by calling a provided function.
//    @param cmd_buffer - reference to the buffer to record commands to.
//    @param function - function containing commands to record.
//    @param render_pass - optional reference to the current renderpass,
//                         this parameter applies only for secondary buffers
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
//    @param framebuffer - optional reference to the current framebuffer,
//                         this parameter applies only for secondary buffers
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
void Device::Record(CmdBuffer& cmd_buffer,
                    ut::Function<void(Context&)> function,
                    ut::Optional<RenderPass&> render_pass,
                    ut::Optional<Framebuffer&> framebuffer)
{
	VkCommandBufferUsageFlags flags = 0;

	if (cmd_buffer.info.usage & CmdBufferInfo::usage_once)
	{
		flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}

	if (cmd_buffer.info.usage & CmdBufferInfo::usage_inside_render_pass)
	{
		flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	}

	VkCommandBufferBeginInfo cmd_buffer_info = {};
	cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buffer_info.pNext = nullptr;
	cmd_buffer_info.flags = flags;

	// inheritance must be set for secondary buffers
	VkCommandBufferInheritanceInfo cmd_buf_inheritance_info = {};
	if (cmd_buffer.info.usage & CmdBufferInfo::level_secondary)
	{
		cmd_buf_inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, cmd_buf_inheritance_info.pNext = NULL;
		cmd_buf_inheritance_info.subpass = 0;
		cmd_buf_inheritance_info.occlusionQueryEnable = VK_FALSE;
		cmd_buf_inheritance_info.queryFlags = 0;
		cmd_buf_inheritance_info.pipelineStatistics = 0;

		// render pass and framebuffer must be set for secondary buffers
		// with CmdBufferInfo::usage_inside_render_pass flag enabled
		if (cmd_buffer.info.usage & CmdBufferInfo::usage_inside_render_pass)
		{
			if (render_pass && framebuffer)
			{
				cmd_buf_inheritance_info.renderPass = render_pass->render_pass.GetVkHandle();
				cmd_buf_inheritance_info.framebuffer = framebuffer->framebuffer.GetVkHandle();
			}
			else
			{
				throw ut::Error(ut::error::invalid_arg, "Vulkan: no renderpass and(or) framebuffer for secondary buffer.");
			}
		}
		else
		{
			cmd_buf_inheritance_info.renderPass = VK_NULL_HANDLE;
			cmd_buf_inheritance_info.framebuffer = VK_NULL_HANDLE;
		}
	}
	else
	{
		cmd_buffer_info.pInheritanceInfo = nullptr;
	}

	// start recording commands
	const VkCommandBuffer cmd = cmd_buffer.buffer.GetVkHandle();
	VkResult res = vkBeginCommandBuffer(cmd, &cmd_buffer_info);
	UT_ASSERT(res == VK_SUCCESS);

	// call function
	Context context(PlatformContext(device.GetVkHandle(), cmd));
	function(context);

	// stop recording
	res = vkEndCommandBuffer(cmd);
	UT_ASSERT(res == VK_SUCCESS);
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
	// check if cmd buffer is a primary buffer
	if (cmd_buffer.info.level != CmdBufferInfo::level_primary)
	{
		throw ut::Error(ut::error::invalid_arg, "Render: secondary buffers can't be submit.");
	}

	// if present queue isn't empty - check if cmd buffer is a dynamic buffer
	const ut::uint32 display_count = static_cast<ut::uint32>(present_queue.GetNum());
	if (display_count != 0 && !(cmd_buffer.info.usage & CmdBufferInfo::usage_once))
	{
		throw ut::Error(ut::error::invalid_arg, "Render: only dynamic command buffers support present.");
	}

	VkSemaphore wait_semaphores[skMaxVkSwapchainPresent];
	VkPipelineStageFlags wait_stages[skMaxVkSwapchainPresent];
	VkSemaphore signal_semaphores[skMaxVkSwapchainPresent];
	
	for (ut::uint32 i = 0; i < display_count; i++)
	{
		Display& display = present_queue[i].Get();

		ut::uint32 swap_id = display.swap_count;
		wait_semaphores[i] = display.availability_semaphores[swap_id].GetVkHandle();
		wait_stages[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		const ut::uint32 current_buffer_id = display.current_buffer_id;
		signal_semaphores[i] = display.present_ready_semaphores[current_buffer_id].GetVkHandle();
	}

	const VkCommandBuffer cmd_bufs[] = { cmd_buffer.buffer.GetVkHandle() };
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = nullptr;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = display_count;
	submit_info[0].pWaitSemaphores = wait_semaphores;
	submit_info[0].pWaitDstStageMask = wait_stages;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = display_count;
	submit_info[0].pSignalSemaphores = signal_semaphores;

	// get fence of the dynamic buffer
	VkFence fence = VK_NULL_HANDLE;
	if (cmd_buffer.info.usage & CmdBufferInfo::usage_once)
	{
		fence = cmd_buffer.fence.GetVkHandle();
		dynamic_cmd_fences.Add(fence);
	}

	// queue the command buffer for execution
	VkResult res = vkQueueSubmit(main_queue.GetVkHandle(), 1, submit_info, fence);
	UT_ASSERT(res == VK_SUCCESS);

	// form swapchain and wait semaphore array
	VkSwapchainKHR swapchains[skMaxVkSwapchainPresent];
	uint32_t swapchain_buffer_indices[skMaxVkSwapchainPresent];
	for (ut::uint32 i = 0; i < display_count; i++)
	{
		const uint32_t id = present_queue[i]->current_buffer_id;
		swapchain_buffer_indices[i] = id;
		swapchains[i] = present_queue[i]->swap_chain.GetVkHandle();
	}

	// enqueue presentation of this frame
	if (display_count != 0)
	{
		// present displays
		VkPresentInfoKHR present;
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.pNext = nullptr;
		present.swapchainCount = display_count;
		present.pSwapchains = swapchains;
		present.pImageIndices = swapchain_buffer_indices;
		present.waitSemaphoreCount = display_count;
		present.pWaitSemaphores = signal_semaphores;
		present.pResults = nullptr;

		// present
		res = vkQueuePresentKHR(main_queue.GetVkHandle(), &present);
		if (res == VK_SUCCESS)
		{
			// enqueue buffer swap for provided displays
			for (ut::uint32 i = 0; i < display_count; i++)
			{
				swap_buffer_queue.Add(present_queue[i].Get());
			}
		}
		else if (res != VK_ERROR_OUT_OF_DATE_KHR && res != VK_SUBOPTIMAL_KHR)
		{
			UT_ASSERT(0);
		}
	}
}

// Call this function to wait on the host for the completion of all
// queue operations for all queues on this device.
void Device::WaitIdle()
{
	vkDeviceWaitIdle(device.GetVkHandle());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//