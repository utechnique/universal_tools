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
// DirectX 11 sampler.
class PlatformSampler
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformSampler(ID3D11SamplerState* sampler_ptr);

	// Move constructor.
	PlatformSampler(PlatformSampler&&) noexcept;

	// Move operator.
	PlatformSampler& operator =(PlatformSampler&&) noexcept;

	// Copying is prohibited.
	PlatformSampler(const PlatformSampler&) = delete;
	PlatformSampler& operator =(const PlatformSampler&) = delete;

private:
	ut::ComPtr<ID3D11SamplerState> sampler_state;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//