//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Camera is means of viewing the 3D scene.
class CameraComponent : public Component
{
public:
	// Explicitly declare defaulted constructors and move operator.
	CameraComponent() = default;
	CameraComponent(CameraComponent&&) = default;
	CameraComponent& operator =(CameraComponent&&) = default;

	// Copying is prohibited.
	CameraComponent(const CameraComponent&) = delete;
	CameraComponent& operator =(const CameraComponent&) = delete;

	// Meta routine.
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	// Each of these functions returns rotated basis vector
	ut::Vector<3> GetDirection(const ut::Quaternion<float>& q) const;
	ut::Vector<3> GetUp(const ut::Quaternion<float>& q) const;
	ut::Vector<3> GetRight(const ut::Quaternion<float>& q) const;

	// view options
	float field_of_view = 90.0f;

	// basis
	static const ut::Vector<3> skDirection;
	static const ut::Vector<3> skUp;
	static const ut::Vector<3> skRight;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//