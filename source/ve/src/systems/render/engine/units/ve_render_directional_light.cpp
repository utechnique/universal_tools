//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_directional_light.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Unit, ve::render::DirectionalLight, "directional_light")
UT_REGISTER_TYPE(ve::render::Resource, ve::render::DirectionalLight::GpuData, "directional_light")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& DirectionalLight::Identify() const
{
	return ut::Identify(this);
}

// Registers light source info into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void DirectionalLight::Reflect(ut::meta::Snapshot& snapshot)
{
	Light::Reflect(snapshot);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//