//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/units/ve_render_view.h"
#include "ve_post_process_view_data.h"
#include "ve_tone_mapping.h"
#include "ve_fxaa.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Post process manager encapsulates post process techniques and applies
// screen-space effects.
class Manager
{
public:
	// Constructor.
	Manager(Toolset& toolset);

	// Creates post-process (per-view) data.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @param format - format of the final image.
	//    @return - a new postprocess::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(ut::uint32 width,
	                                               ut::uint32 height,
	                                               pixel::Format format);

	// Applies all post-process effects to the provided view.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the postprocess::ViewData object containing
	//                  postprocess-specific resources.
	//    @param source - reference to the source image.
	//    @return - optional reference to the final image.
	ut::Optional<Image&> ApplyEffects(Context& context,
	                                  ViewData& data,
	                                  Image& source);

private:
	Toolset& tools;
	ToneMapper tone_mapper;
	Fxaa fxaa;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
