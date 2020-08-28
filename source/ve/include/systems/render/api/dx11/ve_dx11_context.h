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
// DirectX 11 context.
class PlatformContext
{
	friend class Device;
public:
	// Constructor.
	PlatformContext(ID3D11DeviceContext* context_ptr);

	// Move constructor.
	PlatformContext(PlatformContext&&) noexcept;

	// Move operator.
	PlatformContext& operator =(PlatformContext&&) noexcept;

	// Copying is prohibited.
	PlatformContext(const PlatformContext&) = delete;
	PlatformContext& operator =(const PlatformContext&) = delete;

protected:
	// Sets the constant buffers used by the appropriate shader pipeline stage.
	void SetUniformBuffer(ut::uint32 slot, ID3D11Buffer* buffer);


	ut::ComPtr<ID3D11DeviceContext> d3d11_context;
	bool stage_bound[6];
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//