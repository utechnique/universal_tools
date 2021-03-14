//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "components/ve_transform_component.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::Component, ve::TransformComponent, "transform")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
const ut::DynamicType& TransformComponent::Identify() const
{
	return ut::Identify(this);
}

void TransformComponent::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(scale, "scale");
	snapshot.Add(translation, "translation");
	snapshot.Add(rotation, "rotation");
}

// Returns a 4x4 matrix representation.
ut::Matrix<4, 4> TransformComponent::ToMatrix() const
{
	ut::Matrix<3, 3> scale_matrix(0.0f);
	scale_matrix(0, 0) = scale.X();
	scale_matrix(1, 1) = scale.Y();
	scale_matrix(2, 2) = scale.Z();

	ut::Matrix<3, 3> rotation_matrix = scale_matrix * rotation.ToTransform<3>();

	ut::Matrix<4, 4> out;

	out(0, 0) = rotation_matrix(0, 0);
	out(0, 1) = rotation_matrix(0, 1);
	out(0, 2) = rotation_matrix(0, 2);
	out(0, 3) = rotation_matrix(0, 3);

	out(1, 0) = rotation_matrix(1, 0);
	out(1, 1) = rotation_matrix(1, 1);
	out(1, 2) = rotation_matrix(1, 2);
	out(1, 3) = rotation_matrix(1, 3);

	out(2, 0) = rotation_matrix(2, 0);
	out(2, 1) = rotation_matrix(2, 1);
	out(2, 2) = rotation_matrix(2, 2);
	out(2, 3) = rotation_matrix(2, 3);

	out(3, 0) = translation.X();
	out(3, 1) = translation.Y();
	out(3, 2) = translation.Z();
	out(3, 3) = 1.0f;

	return out;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//