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
// Vulkan render target.
class PlatformRenderTarget
{
	friend class Device;
	friend class Context;
	friend class PlatformDisplay;
public:
	// Image view that is used as a render target must be created separately
	// from the shader resource view, because it can affect only one array
	// slice and one mip.
	struct SliceView
	{
		ut::Array< VkRc<vk::image_view> > mips;
	};

	// Constructor.
	PlatformRenderTarget(ut::Array<SliceView> in_slice_views);

	// Move constructor.
	PlatformRenderTarget(PlatformRenderTarget&&) noexcept;

	// Move operator.
	PlatformRenderTarget& operator =(PlatformRenderTarget&&) noexcept;

	// Copying is prohibited.
	PlatformRenderTarget(const PlatformRenderTarget&) = delete;
	PlatformRenderTarget& operator =(const PlatformRenderTarget&) = delete;

private:
	// separate view for each mip
	ut::Array<SliceView> slice_views;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//