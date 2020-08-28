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
// DirectX 11 pipeline state.
class PlatformPipelineState
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformPipelineState(ID3D11InputLayout* input_layout_ptr,
	                      ID3D11RasterizerState* rasterizer_state_ptr,
	                      ID3D11DepthStencilState* depthstencil_state_ptr,
	                      ID3D11BlendState* blend_state_ptr);

	// Move constructor.
	PlatformPipelineState(PlatformPipelineState&&) noexcept;

	// Move operator.
	PlatformPipelineState& operator =(PlatformPipelineState&&) noexcept;

	// Copying is prohibited.
	PlatformPipelineState(const PlatformPipelineState&) = delete;
	PlatformPipelineState& operator =(const PlatformPipelineState&) = delete;

private:
	ut::ComPtr<ID3D11InputLayout> input_layout;
	ut::ComPtr<ID3D11RasterizerState> rasterizer_state;
	ut::ComPtr<ID3D11DepthStencilState> depthstencil_state;
	ut::ComPtr<ID3D11BlendState> blend_state;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//