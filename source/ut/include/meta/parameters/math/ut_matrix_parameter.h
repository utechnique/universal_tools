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
		for (MatrixElementId i = 0; i < rows; i++)
		{
			for (MatrixElementId j = 0; j < columns; j++)
			{
				Optional<String> element_name = GetElementName(j);
				if (element_name)
				{
					snapshot.Add(matrix(i, j), element_name.Move());
				}
				else
				{
					snapshot << matrix(i, j);
				}
			}
		}
	}

private:
	// Helper function to generate element's name (such as 'x', 'y', etc.)
	Optional<String> GetElementName(MatrixElementId id)
	{
		// makes sense only for vectors
		if (rows != 1)
		{
			return Optional<String>();
		}

		switch (id)
		{
			case 0: return String("x");
			case 1: return String("y");
			case 2: return String("z");
			case 3: return String("w");
		}

		return Optional<String>();
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//