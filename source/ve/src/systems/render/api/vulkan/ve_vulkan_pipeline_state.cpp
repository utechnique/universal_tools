//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_pipeline_state.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformPipelineState::PlatformPipelineState(VkDevice device_handle,
                                             VkPipeline pipeline_handle,
                                             VkPipelineLayout layout_handle,
                                             VkDescriptorSetLayout dsl_handle) : pipeline(pipeline_handle, device_handle)
                                                                               , layout(layout_handle, device_handle)
                                                                               , dsl(dsl_handle, device_handle)
{}

// Move constructor.
PlatformPipelineState::PlatformPipelineState(PlatformPipelineState&&) noexcept = default;

// Move operator.
PlatformPipelineState& PlatformPipelineState::operator =(PlatformPipelineState&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//