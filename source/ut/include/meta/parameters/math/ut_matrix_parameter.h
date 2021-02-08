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
// Returns name for the matrix elements. Some matrix types can have 'named'
// elements such as 'x,y,z..' or 'r,g,b..'
template<typename Tag>
inline Optional<String> GetMatrixElementName(MatrixElementId id)
{
	return Optional<String>();
}

//----------------------------------------------------------------------------//
// ut::Parameter<Matrix> is a template specialization for matrices.
template<MatrixElementId rows, MatrixElementId columns, typename Scalar, typename Tag>
class Parameter< Matrix<rows, columns, Scalar, Tag> > : public BaseParameter
{
	typedef Matrix<rows, columns, Scalar, Tag> MatrixType;
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
				Optional<String> element_name = GetMatrixElementName<Tag>(j);
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
};

// Specialization for vectors.
template<> inline Optional<String> GetMatrixElementName<MatrixVectorTag>(MatrixElementId id)
{
	switch (id)
	{
	case 0: return String("x");
	case 1: return String("y");
	case 2: return String("z");
	case 3: return String("w");
	}

	return Optional<String>();
}

// Specialization for colors.
template<> inline Optional<String> GetMatrixElementName<MatrixColorTag>(MatrixElementId id)
{
	switch (id)
	{
	case 0: return String("r");
	case 1: return String("g");
	case 2: return String("b");
	case 3: return String("a");
	}

	return Optional<String>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//