//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "ut.h"
#include <d3d11.h>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// DirectX 11 command buffer.
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
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//