//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
#include "systems/render/api/vulkan/ve_vulkan_cmd_buffer.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan context.
class PlatformContext
{
	friend class Device;
public:
	// Constructor.
	PlatformContext(VkDevice device_handle,
	                PlatformCmdBuffer& cmd_buffer_ref);

	// Move constructor.
	PlatformContext(PlatformContext&&) noexcept;

	// Move operator is prohibited.
	PlatformContext& operator =(PlatformContext&&) noexcept = delete;

	// Copying is prohibited.
	PlatformContext(const PlatformContext&) = delete;
	PlatformContext& operator =(const PlatformContext&) = delete;

protected:
	VkDevice device;
	PlatformCmdBuffer& cmd_buffer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//