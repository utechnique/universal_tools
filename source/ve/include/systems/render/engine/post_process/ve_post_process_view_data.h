//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_post_process_slots.h"
#include "ve_tone_mapping.h"
#include "ve_stencil_highlight.h"
#include "ve_fxaa.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// ve::render::postprocess::ViewData encapsulates render targets, uniform
// buffers, pipelines and other resources needed to apply post-process effects
// on a ve::render::View object.
class ViewData
{
public:
	// Constructor.
	ViewData(ut::Array<SwapSlot> in_swap_slots,
	         RenderPass in_color_only_pass,
	         RenderPass in_color_and_ds_pass,
             RenderPass in_clear_color_and_ds_pass,
	         ToneMapper::ViewData in_tone_mapping,
	         StencilHighlight::ViewData in_stencil_highlight,
	         Fxaa::ViewData in_fxaa);

	// Move constructor and operator.
	ViewData(ViewData&&) = default;
	ViewData& operator =(ViewData&&) = default;

	// Copying is prohibited.
	ViewData(const ViewData&) = delete;
	ViewData& operator =(const ViewData&) = delete;

	// intermediate buffers
	SwapManager swap_mgr;

	// render passes for all effects
	RenderPass color_only_pass;
	RenderPass color_and_ds_pass;
	RenderPass clear_color_and_ds_pass;

	// effects
	ToneMapper::ViewData tone_mapping;
	StencilHighlight::ViewData stencil_highlight;
	Fxaa::ViewData fxaa;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
