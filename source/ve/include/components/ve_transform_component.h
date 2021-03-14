//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Transform composed of Scale, Rotation(as a quaternion), and Translation.
// It can be used to convert from one space to another.
// Transformation is applied in the order: Scale->Rotate->Translate.
class TransformComponent : public Component
{
public:
	// Explicitly declare defaulted constructors and move operator.
	TransformComponent() = default;
	TransformComponent(TransformComponent&&) = default;
	TransformComponent& operator =(TransformComponent&&) = default;

	// Copying is prohibited.
	TransformComponent(const TransformComponent&) = delete;
	TransformComponent& operator =(const TransformComponent&) = delete;

	// Meta routine.
	const ut::DynamicType& Identify() const;
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