//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/units/ve_render_view.h"
#include "ve_deferred_shading.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Lighting manager encapsulates different lighting techniques.
class Manager
{
public:
	// Constructor.
	Manager(Toolset& toolset);

	// Creates lighting (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @return - a new lighting::ViewData object or error if failed.
	ut::Result<lighting::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                         ut::uint32 width,
	                                                         ut::uint32 height);

private:
	Toolset& tools;

public:
	DeferredShading deferred_shading;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
