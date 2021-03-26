//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_model.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Unit, ve::render::Model, "render_model")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& Model::Identify() const
{
	return ut::Identify(this);
}

// Registers this model unit into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void Model::Reflect(ut::meta::Snapshot& snapshot)
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//