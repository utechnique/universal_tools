//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_unit.h"
#include "systems/render/ve_render_api.h"
#include "systems/render/resources/ve_render_mesh.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Model is a compact general-purpose unit.
class Model : public Unit
{
public:
	// Explicitly declare defaulted constructors and move operator.
	Model() = default;
	Model(Model&&) = default;
	Model& operator =(Model&&) = default;

	// Copying is prohibited.
	Model(const Model&) = delete;
	Model& operator =(const Model&) = delete;

	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::Array< RcRef<Mesh> > lods;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//