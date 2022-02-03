//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
#include "ve_transform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Transform composed of Scale, Rotation(as a quaternion), and Translation.
// It can be used to convert from one space to another.
// Transformation is applied in the order: Scale->Rotate->Translate.
class TransformComponent : public Component, public Transform
{
public:
	// Explicitly declare defaulted constructors and move operator.
	TransformComponent() = default;
	TransformComponent(TransformComponent&&) = default;
	TransformComponent& operator =(TransformComponent&&) = default;

	// Copying is prohibited.
	TransformComponent(const TransformComponent&) = delete;
	TransformComponent& operator =(const TransformComponent&) = delete;

	// Identify() method must be implemented for polymorphic types.
	const ut::DynamicType& Identify() const override;

	// Registers transform info into the reflection tree.
	//    @param snapshot - reference to the reflection tree.
	void Reflect(ut::meta::Snapshot& snapshot);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//