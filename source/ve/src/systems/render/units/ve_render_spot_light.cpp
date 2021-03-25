//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/units/ve_render_spot_light.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Unit, ve::render::SpotLight, "spot_light")
UT_REGISTER_TYPE(ve::render::Resource, ve::render::SpotLight::GpuData, "spot_light")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& SpotLight::Identify() const
{
	return ut::Identify(this);
}

// Registers light source info into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void SpotLight::Reflect(ut::meta::Snapshot& snapshot)
{
	Light::Reflect(snapshot);
	snapshot.Add(inner_cone, "inner_cone");
	snapshot.Add(outer_cone, "outer_cone");
	snapshot.Add(attenuation_distance, "attenuation_distance");
	snapshot.Add(shape_radius, "shape_radius");
	snapshot.Add(shape_length, "shape_length");
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//