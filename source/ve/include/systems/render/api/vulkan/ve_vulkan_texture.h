//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan texture.
class PlatformTexture
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	explicit PlatformTexture();

	// Move constructor.
	PlatformTexture(PlatformTexture&&) noexcept;

	// Move operator.
	PlatformTexture& operator =(PlatformTexture&&) noexcept;

	// Copying is prohibited.
	PlatformTexture(const PlatformTexture&) = delete;
	PlatformTexture& operator =(const PlatformTexture&) = delete;

private:

};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//