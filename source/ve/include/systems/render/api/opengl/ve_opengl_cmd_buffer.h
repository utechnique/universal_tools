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
// OpenGL command buffer.
class PlatformCmdBuffer
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformCmdBuffer();

	// Move constructor.
	PlatformCmdBuffer(PlatformCmdBuffer&&) noexcept;

	// Move operator.
	PlatformCmdBuffer& operator =(PlatformCmdBuffer&&) noexcept;

	// Copying is prohibited.
	PlatformCmdBuffer(const PlatformCmdBuffer&) = delete;
	PlatformCmdBuffer& operator =(const PlatformCmdBuffer&) = delete;

private:
	ut::Optional< ut::Function<void(Context&)> > proc;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//