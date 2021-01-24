//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Free camera controller gives a user
// the ability to control a camera directly.
class FreeCameraControllerComponent : public Component
{
public:
	// Explicitly declare defaulted constructors and move operator.
	FreeCameraControllerComponent() = default;
	FreeCameraControllerComponent(FreeCameraControllerComponent&&) = default;
	FreeCameraControllerComponent& operator =(FreeCameraControllerComponent&&) = default;

	// Copying is prohibited.
	FreeCameraControllerComponent(const FreeCameraControllerComponent&) = delete;
	FreeCameraControllerComponent& operator =(const FreeCameraControllerComponent&) = delete;

	// Meta routine.
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	float speed = 5.0f; // per second
	float sensitivity = 0.25f;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//