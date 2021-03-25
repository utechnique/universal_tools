//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit_mgr.h"
#include "systems/render/engine/ve_render_model_batcher.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Model Policy.
template<> class Policy<Model>
{
public:
	// Per-frame gpu resources.
	struct FrameData
	{
		ut::Array<Model::Batch> batches;
	};

	// Constructor.
	Policy(Toolset &toolset_ref,
	       UnitSelector& unit_selector,
	       Policies& policy_mgr);

	// Initializes a provided model unit.
	void Initialize(Model& model);

	// batcher gathers model units into batches
	ModelBatcher batcher;

private:
	Toolset &tools;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
