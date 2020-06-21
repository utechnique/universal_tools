//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "math/ut_rect.h"
#include "meta/parameters/math/ut_matrix_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Matrix> is a template specialization for matrices.
template<typename Scalar>
class Parameter< Rect<Scalar> > : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed matrix (vector)
	Parameter(Rect<Scalar>* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< Rect<Scalar> >();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		Rect<Scalar>& rect = *static_cast<Rect<Scalar>*>(ptr);

		// register all elements
		snapshot.Add(rect.offset, "offset");
		snapshot.Add(rect.extent, "extent");
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//