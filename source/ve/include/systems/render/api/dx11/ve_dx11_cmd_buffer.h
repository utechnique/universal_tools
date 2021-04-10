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
	PlatformCmdBuffer(ID3D11CommandList* in_cmd_list,
	                  ID3D11DeviceContext* in_deferred_context);

	// Move constructor.
	PlatformCmdBuffer(PlatformCmdBuffer&&) noexcept;

	// Move operator.
	PlatformCmdBuffer& operator =(PlatformCmdBuffer&&) noexcept;

	// Copying is prohibited.
	PlatformCmdBuffer(const PlatformCmdBuffer&) = delete;
	PlatformCmdBuffer& operator =(const PlatformCmdBuffer&) = delete;

private:
	ut::ComPtr<ID3D11CommandList> cmd_list;
	ut::ComPtr<ID3D11DeviceContext> deferred_context;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//