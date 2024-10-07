//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit_mgr.h"
#include "systems/render/engine/ve_render_batcher.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Mesh Instance Policy.
template<> class Policy<MeshInstance>
{
public:
	// Per-frame gpu resources.
	struct FrameData
	{
		ut::Array<MeshInstance::Batch> batches;
	};

	// Constructor.
	Policy(Toolset &toolset_ref,
	       UnitSelector& unit_selector,
	       Policies& policy_mgr);

	// Initializes a provided mesh instance unit.
	void Initialize(MeshInstance& instance);

	// Batcher gathers drawcalls into batches
	Batcher batcher;

private:
	Toolset &tools;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
