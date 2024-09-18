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

	// Projection types
	enum Projection
	{
		perspective_projection,
		orthographic_projection
	};

	// view options
	Projection projection = perspective_projection;

	// frustum properties (z-near, z-far, aspect ratio, etc.)
	float near_plane = 0.1f;
	float far_plane = 1e+5f;
	float aspect_ratio = 2.0f;

	// projection-specific properties
	float field_of_view = 60.0f; // for perspective projection
	float width = 10.0f; // for orthographic projection

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