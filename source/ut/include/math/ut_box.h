//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "math/ut_matrix.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// 3D box.
template<typename Scalar = float>
struct Box
{
	Box()
	{}

	Box(Scalar offset_x, Scalar offset_y, Scalar offset_z,
	    Scalar width, Scalar height, Scalar depth) : offset(offset_x, offset_y, offset_z)
	                                               , extent(width, height, depth)
	{}

	Vector<3, Scalar> GetCenter() const
	{
		return offset + extent.ElementWise() / static_cast<Scalar>(2);
	}

	Vector<3, Scalar> offset;
	Vector<3, Scalar> extent;
};

// Specialized type name function for a box
template <typename Scalar>
struct Type< Box<Scalar> >
{
	static inline const char* Name() { return "box"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//