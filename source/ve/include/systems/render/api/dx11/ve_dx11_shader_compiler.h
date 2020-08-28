//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// DirectX11 shader compiler.
class PlatformShaderCompiler
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformShaderCompiler();

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
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//