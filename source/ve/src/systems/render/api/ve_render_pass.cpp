//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_pass.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
RenderPass::RenderPass(PlatformRenderPass platform_render_pass,
                       ut::Array<RenderTargetSlot> in_color_slots,
                       ut::Optional<RenderTargetSlot> in_depth_stencil_slot) : PlatformRenderPass(ut::Move(platform_render_pass))
                                                                             , color_slots(ut::Move(in_color_slots))
                                                                             , depth_stencil_slot(ut::Move(in_depth_stencil_slot))
{}

// Move constructor.
RenderPass::RenderPass(RenderPass&&) noexcept = default;

// Move operator.
RenderPass& RenderPass::operator =(RenderPass&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//