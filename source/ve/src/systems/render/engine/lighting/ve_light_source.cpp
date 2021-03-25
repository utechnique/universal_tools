//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/lighting/ve_light_source.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Original direction of the light source. Final direction is calculated as
// skDirection * world_transform.
const ut::Vector<3> Light::skDirection(1, 0, 0);

// This vector is the original shape direction (before transformation)
// in the case if this light source represents a tube (@shape_length > 0). 
const ut::Vector<3> Light::skTubeDirection(0, 0, 1);

//----------------------------------------------------------------------------//
// Registers light source info into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void Light::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(color, "color");
	snapshot.Add(intensity, "intensity");
}

// Direction of the light source is calculated as @rotation * skDirection.
ut::Vector<3> Light::GetDirection(const ut::Quaternion<float>& rotation)
{
	return rotation.Rotate(skDirection);
}

// Returns @rotation * skTubeDirection.
ut::Vector<3> Light::GetTubeDirection(const ut::Quaternion<float>& rotation)
{
	return rotation.Rotate(skTubeDirection);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//