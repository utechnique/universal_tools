//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_pipeline_state.h"
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformPipelineState::PlatformPipelineState(ID3D11InputLayout* input_layout_ptr,
                                             ID3D11RasterizerState* rasterizer_state_ptr,
                                             ID3D11DepthStencilState* depthstencil_state_ptr,
                                             ID3D11BlendState* blend_state_ptr) : input_layout(input_layout_ptr)
                                                                                , blend_state(blend_state_ptr)
                                                                                , rasterizer_state(rasterizer_state_ptr)
                                                                                , depthstencil_state(depthstencil_state_ptr)
{}

// Move constructor.
PlatformPipelineState::PlatformPipelineState(PlatformPipelineState&&) noexcept = default;

// Move operator.
PlatformPipelineState& PlatformPipelineState::operator =(PlatformPipelineState&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//