//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/policy/ve_render_model_policy.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Policy<Model>::Policy(Toolset &toolset,
                      UnitSelector& unit_selector,
                      Policies& engine_policies) : tools(toolset)
                                                 , batcher(toolset)
{}


// Initializes a provided model unit.
void Policy<Model>::Initialize(Model& model)
{
	// load mesh
	ut::Result<RcRef<Mesh>, ut::Error> mesh = tools.rc_mgr.Find<Mesh>(model.mesh_path);
	if (!mesh)
	{
		throw ut::Error(ut::error::not_found);
	}
	model.mesh = mesh.Move();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//