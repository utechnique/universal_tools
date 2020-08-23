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
// Vulkan shader compiler.
class PlatformShaderCompiler
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	explicit PlatformShaderCompiler();

	// Move constructor.
	PlatformShaderCompiler(PlatformShaderCompiler&&) noexcept;

	// Move operator.
	PlatformShaderCompiler& operator =(PlatformShaderCompiler&&) noexcept;

	// Copying is prohibited.
	PlatformShaderCompiler(const PlatformShaderCompiler&) = delete;
	PlatformShaderCompiler& operator =(const PlatformShaderCompiler&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//