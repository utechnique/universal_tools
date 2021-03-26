//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/units/ve_render_point_light.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Unit, ve::render::PointLight, "point_light")
UT_REGISTER_TYPE(ve::render::Resource, ve::render::PointLight::GpuData, "point_light")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& PointLight::Identify() const
{
	return ut::Identify(this);
}

// Registers light source info into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void PointLight::Reflect(ut::meta::Snapshot& snapshot)
{
	Light::Reflect(snapshot);
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