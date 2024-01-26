//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_ambient_light.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Unit, ve::render::AmbientLight, "ambient_light")
UT_REGISTER_TYPE(ve::render::Resource, ve::render::AmbientLight::GpuData, "ambient_light")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& AmbientLight::Identify() const
{
	return ut::Identify(this);
}

// Registers light source info into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void AmbientLight::Reflect(ut::meta::Snapshot& snapshot)
{
	Light::Reflect(snapshot);
	snapshot.Add(attenuation_distance, "attenuation_distance");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//