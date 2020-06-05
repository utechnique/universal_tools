//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
#include "systems/render/api/opengl/ve_opengl_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// OpenGL renderpass.
class PlatformRenderPass
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformRenderPass();

	// Move constructor.
	PlatformRenderPass(PlatformRenderPass&&) noexcept;

	// Move operator.
	PlatformRenderPass& operator =(PlatformRenderPass&&) noexcept;

	// Copying is prohibited.
	PlatformRenderPass(const PlatformRenderPass&) = delete;
	PlatformRenderPass& operator =(const PlatformRenderPass&) = delete;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//