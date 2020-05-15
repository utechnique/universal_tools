//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "math/ut_matrix.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Matrix> is a template specialization for matrices.
template<MatrixElementId rows, MatrixElementId columns, typename Scalar>
class Parameter< Matrix<rows, columns, Scalar> > : public BaseParameter
{
	typedef Matrix<rows, columns, Scalar> MatrixType;
public:
	// Constructor
	//    @param p - pointer to the managed matrix (vector)
	Parameter(MatrixType* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName<MatrixType>();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		// get array reference from pointer
		MatrixType& matrix = *static_cast<MatrixType*>(ptr);

		// register all elements
		Scalar(&elements)[rows*columns] = *reinterpret_cast<Scalar(*)[rows*columns]>(matrix.GetData());
		snapshot << elements;
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//