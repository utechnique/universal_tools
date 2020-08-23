//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_pipeline_state.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PipelineState::PipelineState(PlatformPipelineState platform_pipeline,
                             PipelineState::Info pipeline_info) : PlatformPipelineState(ut::Move(platform_pipeline))
                                                                , info(ut::Move(pipeline_info))
{}

// Move constructor.
PipelineState::PipelineState(PipelineState&&) noexcept = default;

// Move operator.
PipelineState& PipelineState::operator =(PipelineState&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//