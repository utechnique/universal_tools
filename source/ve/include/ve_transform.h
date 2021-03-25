//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Transform composed of Scale, Rotation(as a quaternion), and Translation.
// It can be used to convert from one space to another.
// Transformation is applied in the order: Scale->Rotate->Translate.
class Transform : public ut::meta::Reflective
{
public:
	// Returns multiplied transform of 2 transforms.
	Transform operator*(const Transform& right) const;

	// Multiplication assignment operator.
	Transform& operator*=(const Transform& right);

	// Registers transform info into the reflection tree.
	//    @param snapshot - reference to the reflection tree.
	void Reflect(ut::meta::Snapshot& snapshot);

	// Returns a 4x4 matrix representation.
	ut::Matrix<4, 4> ToMatrix() const;

	ut::Vector<3> scale = ut::Vector<3>(1, 1, 1);
	ut::Vector<3> translation = ut::Vector<3>(0, 0, 0);
	ut::Quaternion<float> rotation;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//