//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/policy/ve_render_mesh_instance_policy.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Policy<MeshInstance>::Policy(Toolset &toolset,
                      UnitSelector& unit_selector,
                      Policies& engine_policies) : tools(toolset)
                                                 , batcher(toolset)
{}


// Initializes a provided mesh instance unit.
void Policy<MeshInstance>::Initialize(MeshInstance& instance)
{
	// load mesh resource
	ut::Result<RcRef<Mesh>, ut::Error> mesh = tools.rc_mgr.Find<Mesh>(instance.mesh_path);
	if (!mesh)
	{
		throw ut::Error(ut::error::not_found);
	}
	instance.mesh = mesh.Move();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//