//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/lighting/ve_lighting_manager.h"
#include "systems/render/engine/policy/ve_render_model_policy.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
Manager::Manager(Toolset& toolset) : tools(toolset), deferred_shading(toolset)
{}

// Creates lighting (per-view) data.
//    @param depth_stencil - reference to the depth buffer.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @return - a new lighting::ViewData object or error if failed.
ut::Result<lighting::ViewData, ut::Error> Manager::CreateViewData(Target& depth_stencil,
                                                                  ut::uint32 width,
                                                                  ut::uint32 height)
{
	ut::Result<DeferredShading::ViewData, ut::Error> def_sh_data = deferred_shading.CreateViewData(depth_stencil,
	                                                                                               width, height);
	if (!def_sh_data)
	{
		return ut::MakeError(def_sh_data.MoveAlt());
	}

	return lighting::ViewData(def_sh_data.Move());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//