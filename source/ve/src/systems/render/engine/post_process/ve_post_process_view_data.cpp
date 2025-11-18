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
                   ToneMapper::ViewData in_tone_mapping,
                   StencilHighlight::ViewData in_stencil_highlight,
                   Dithering::ViewData in_dithering,
                   Fxaa::ViewData in_fxaa) : swap_mgr(ut::Move(in_swap_slots))
                                           , tone_mapping(ut::Move(in_tone_mapping))
                                           , stencil_highlight(ut::Move(in_stencil_highlight))
                                           , dithering(ut::Move(in_dithering))
                                           , fxaa(ut::Move(in_fxaa))
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//