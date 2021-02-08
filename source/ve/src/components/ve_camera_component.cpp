//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "components/ve_camera_component.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::Component, ve::CameraComponent, "camera")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
const ut::Vector<3> CameraComponent::skDirection = ut::Vector<3>(1, 0, 0);
const ut::Vector<3> CameraComponent::skUp = ut::Vector<3>(0, 1, 0);
const ut::Vector<3> CameraComponent::skRight = ut::Vector<3>(0, 0, -1);

//----------------------------------------------------------------------------->
const ut::DynamicType& CameraComponent::Identify() const
{
	return ut::Identify(this);
}

void CameraComponent::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(near_plane, "near_plane");
	snapshot.Add(far_plane, "far_plane");
	snapshot.Add(aspect_ratio, "aspect_ratio");
	snapshot.Add(field_of_view, "field_of_view");
	snapshot.Add(width, "width");
}

//----------------------------------------------------------------------------->

ut::Vector<3> CameraComponent::GetDirection(const ut::Quaternion<float>& q) const
{
	return q.Rotate(skDirection);
}

ut::Vector<3> CameraComponent::GetUp(const ut::Quaternion<float>& q) const
{
	return q.Rotate(skUp);
}

ut::Vector<3> CameraComponent::GetRight(const ut::Quaternion<float>& q) const
{
	return q.Rotate(skRight);
}
//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//