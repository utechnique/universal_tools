//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit_mgr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Directional Light Policy.
template<> class Policy<DirectionalLight>
{
public:
	// Constructor.
	Policy(Toolset &toolset_ref,
	       UnitSelector& unit_selector,
	       Policies& policy_mgr);

	// Initializes a provided light unit.
	void Initialize(DirectionalLight& light);

private:
	Toolset &tools;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
