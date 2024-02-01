//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_post_process_view_data.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Constructor.
ViewData::ViewData(ut::Array<SwapSlot> in_swap_slots,
                   RenderPass in_color_only_pass,
                   RenderPass in_color_and_ds_pass,
                   RenderPass in_clear_color_and_ds_pass,
                   ToneMapper::ViewData in_tone_mapping,
                   StencilHighlight::ViewData in_stencil_highlight,
                   Fxaa::ViewData in_fxaa) : swap_mgr(ut::Move(in_swap_slots))
                                           , color_only_pass(ut::Move(in_color_only_pass))
                                           , color_and_ds_pass(ut::Move(in_color_and_ds_pass))
                                           , clear_color_and_ds_pass(ut::Move(in_clear_color_and_ds_pass))
                                           , tone_mapping(ut::Move(in_tone_mapping))
                                           , stencil_highlight(ut::Move(in_stencil_highlight))
                                           , fxaa(ut::Move(in_fxaa))
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//