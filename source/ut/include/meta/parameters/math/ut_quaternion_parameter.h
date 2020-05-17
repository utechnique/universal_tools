//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "math/ut_quaternion.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Matrix> is a template specialization for matrices.
template<typename Scalar>
class Parameter< Quaternion<Scalar> > : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed matrix (vector)
	Parameter(Quaternion<Scalar>* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< Quaternion<Scalar> >();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		Quaternion<Scalar>& quaternion = *static_cast<Quaternion<Scalar>*>(ptr);

		// register all elements
		snapshot.Add(quaternion.r, "r");
		snapshot.Add(quaternion.i, "i");
		snapshot.Add(quaternion.j, "j");
		snapshot.Add(quaternion.k, "k");
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//