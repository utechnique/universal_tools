//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_transform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Returns multiplied transform of 2 transforms.
Transform Transform::operator*(const Transform& right) const
{
	Transform out;
	const ut::Vector<3> scaled_translate = right.translation.ElementWise() * scale;
	out.scale = scale.ElementWise() * right.scale;
	out.rotation = rotation * right.rotation;
	out.translation = translation + rotation.Rotate(scaled_translate);
	return out;
}

// Multiplication assignment operator.
Transform& Transform::operator*=(const Transform& right)
{
	*this = this->operator*(right);
	return *this;
}

// Registers transform info into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void Transform::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(scale, "scale");
	snapshot.Add(translation, "translation");
	snapshot.Add(rotation, "rotation");
}

// Returns a 4x4 matrix representation.
ut::Matrix<4, 4> Transform::ToMatrix() const
{
	ut::Matrix<4, 4> scale_matrix(0.0f);
	scale_matrix(0, 0) = scale.X();
	scale_matrix(1, 1) = scale.Y();
	scale_matrix(2, 2) = scale.Z();
	scale_matrix(3, 3) = 1.0f;

	ut::Matrix<4, 4> out = scale_matrix * rotation.ToTransform<4>();
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