//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "components/ve_camera_component.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::Component, ve::CameraComponent, "camera")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
const ut::DynamicType& CameraComponent::Identify() const
{
	return ut::Identify(this);
}

void CameraComponent::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(position, "position");
	snapshot.Add(direction, "direction");
	snapshot.Add(tilt, "tilt");
	snapshot.Add(field_of_view, "field_of_view");
}
//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//