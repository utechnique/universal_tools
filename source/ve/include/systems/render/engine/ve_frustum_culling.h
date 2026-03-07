//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_render_toolset.h"
#include "ve_transform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Performs frustum culling optimization.
template<typename Scalar>
class FrustumCulling
{
public:
	// Frustum planes indices.
	enum Plane : ut::uint32
	{
		left,
		right,
		bottom,
		top,
		znear,
		zfar,
		count
	};

	// Constructor. Initializes frustum planes from the provided
	// view-projection matrix.
	FrustumCulling(const ut::Matrix<4, 4, Scalar>& in_view_proj) : view_proj(in_view_proj)
	{
		// extract frustum planes from the view projection matrix
		for (int i = 0; i < 4; i++)
		{
			planes[Plane::left](0, i) = view_proj(i, 3) + view_proj(i, 0);
			planes[Plane::right](0, i) = view_proj(i, 3) - view_proj(i, 0);
			planes[Plane::bottom](0, i) = view_proj(i, 3) + view_proj(i, 1);
			planes[Plane::top](0, i) = view_proj(i, 3) - view_proj(i, 1);
			planes[Plane::znear](0, i) = view_proj(i, 2);
			planes[Plane::zfar](0, i) = view_proj(i, 3) - view_proj(i, 2);
		}

		// normalize frustum planes
		for (int i = 0; i < 6; i++)
		{
			ut::Vector<4, Scalar>& plane = planes[i];
			const ut::Vector<3, Scalar> n(plane.X(), plane.Y(), plane.Z());
			const Scalar mag = n.Length();
			plane = plane.ElementWise() / mag;
		}
	}

	// Calculates the distance from a given point to the frustum plane
	// defined by its index. The division by the magnitude is omitted since all
	// frustum planes are normalized.
	Scalar DistanceToPlane(Plane plane_index, const ut::Vector<3, Scalar>& point) const
	{
		const ut::Vector<4, Scalar>& plane = planes[plane_index];

		return plane.X() * point.X() +
		       plane.Y() * point.Y() +
		       plane.Z() * point.Z() +
		       plane.W();
	}

	// Returns true if the provided sphere must be culled.
	//    @param sphere - a reference to the sphere to pass the cull test.
	//    @param transform - a reference to the ve::Transform object to be
	//                       applied to the @sphere.
	//    @return - true if the @sphere must be culled.
	bool CullSphere(const ut::Sphere<Scalar>& sphere,
	                const ve::Transform& transform) const
	{
		// transform the provided sphere
		const Scalar scale = ut::Max(ut::Max(transform.scale.X(),
		                                     transform.scale.Y()),
		                             transform.scale.Z());
		const ut::Vector<3, Scalar> center = transform.rotation.Rotate(sphere.center.ElementWise() * scale) +
		                                     transform.translation;
		const Scalar radius = sphere.radius * scale;

		// calculate the minimum distance from the
		// center of the sphere to the frustum
		const Scalar dist_left_right = ut::Min(DistanceToPlane(Plane::left, center),
		                                       DistanceToPlane(Plane::right, center));
		const Scalar dist_bottom_top = ut::Min(DistanceToPlane(Plane::bottom, center),
		                                       DistanceToPlane(Plane::top, center));
		const Scalar dist_near_far = ut::Min(DistanceToPlane(Plane::znear, center),
		                                     DistanceToPlane(Plane::zfar, center));
		const Scalar min_dist = ut::Min(ut::Min<Scalar>(dist_left_right,
		                                                dist_bottom_top),
		                                dist_near_far);

		// the sphere must be culled if the minimum distance from its center to
		// the frustum is less than the negative radius of the sphere
		return min_dist + radius + ut::Precision<Scalar>::epsilon < 0.0f;
	}

private:
	ut::Matrix<4, 4, Scalar> view_proj;
	ut::Vector<4, Scalar> planes[Plane::count];
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
