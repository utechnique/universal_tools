//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "math/ut_cmp.h"
#include "math/ut_pow.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Type of element's index inside a matrix.
typedef uint32 MatrixElementId;

// ut::Matrix is a complete template class for matrices. It can represent both
// matrix and vector (as a matrix with only one row).
template<MatrixElementId rows, MatrixElementId columns, typename ElementType = float>
class Matrix
{
private:
	// Number of elements inside this matrix.
	static constexpr MatrixElementId size = rows * columns;

public:
	// "Named Constructor" to generate identity matrix.
	static Matrix MakeIdentity()
	{
		static_assert(rows == columns, "Only square matrix can be identity.");
		Matrix out(static_cast<ElementType>(0));
		for (MatrixElementId i = 0; i < columns; i++)
		{
			out(i, i) = static_cast<ElementType>(1);
		}
		return out;
	}

	// Constructor. Elements are not initialized.
	Matrix()
	{}

	// Constructor. Elements are initialized with
	// values from the provided array.
	Matrix(ElementType data[size])
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = data[i];
		}
	}

	// Constructor. All elements are initialized with a scalar value.
	explicit Matrix(ElementType scalar)
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = scalar;
		}
	}

	// Constructor. Initializes elements from the provided arguments.
	// Number of arguments must be exactly the same as number of elements in this matrix.
	// Each argument must be convertible to @ElementType.
	template<typename... Elements>
	Matrix(Elements... elements)
	{
		// convert all arguments to the desired type
		ElementType args[]{ static_cast<ElementType>(elements)... };

		// check if number of provided arguments is exactly
		// the same as number of elements in the matrix
		static_assert(sizeof(args) / sizeof(ElementType) == size, "Invalid number of arguments");
		
		// set elements
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = args[i];
		}
	}

	// Copy constructor, just copies all elements one by one.
	Matrix(const Matrix& copy)
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = copy.table[i];
		}
	}

	// Assignment operator, all elements are set to the provided scalar value.
	Matrix& operator = (ElementType scalar)
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = scalar;
		}
		return *this;
	}

	// Assignment operator, just copies all elements one by one.
	Matrix& operator = (const Matrix& copy)
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = copy.table[i];
		}
		return *this;
	}

	// Returns a constant reference to the element that is defined by it's row and column.
	//    @param row - index of the row desired element belongs to.
	//    @param column - index of the column desired element belongs to.
	//    @return - constant reference to the desired element.
	const ElementType& operator()(MatrixElementId row, MatrixElementId column) const
	{
		UT_ASSERT(row * column < size);
		return table[row*columns + column];
	}

	// Returns a reference to the element that is defined by it's row and column.
	//    @param row - index of the row desired element belongs to.
	//    @param column - index of the column desired element belongs to.
	//    @return - reference to the desired element.
	ElementType& operator()(MatrixElementId row, MatrixElementId column)
	{
		UT_ASSERT(row * column < size);
		return table[row*columns + column];
	}

	// Comparison operator, each element is compared one by one.
	//    @param other - matrix to compare with.
	bool operator == (const Matrix& other) const
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			if (!Equal(table[i], other.table[i]))
			{
				return false;
			}
		}
		return true;
	}

	// Negation comparison operator, each element is compared one by one.
	//    @param other - matrix to compare with.
	bool operator != (const Matrix& other) const
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			if (!Equal(table[i], other.table[i]))
			{
				return true;
			}
		}
		return false;
	}

	// Negation operator.
	// Changes sign of every element in the matrix.
	Matrix operator - () const
	{
		Matrix<rows, columns, ElementType> out;
		for (MatrixElementId i = 0; i < size; i++)
		{
			out.table[i] = -table[i];
		}
		return out;
	}

	// Addition assignment operator. Addition is performed for each pair
	// of elements individually.
	Matrix& operator += (const Matrix& right)
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] += right.table[i];
		}
		return *this;
	}

	// Subtraction assignment operator. Subtraction is performed for each pair
	// of elements individually.
	Matrix& operator -= (const Matrix& right)
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] -= right.table[i];
		}
		return *this;
	}

	// Addition operator.
	Matrix operator + (const Matrix& right) const
	{
		Matrix out(*this);
		out += right;
		return out;
	}

	// Subtraction operator.
	Matrix operator - (const Matrix& right) const
	{
		Matrix out(*this);
		out -= right;
		return out;
	}

	// Multiplication operator accepts only square matrices, where side of the
	// provided matrix must be equal to the number of columns in the current
	// one. Use Muliple() method to multiply matrices with custom size.
	Matrix operator * (const Matrix<columns, columns, ElementType>& right) const
	{
		return Multiply<columns>(right);
	}

	// Multiplies current matrix with the provided one. Number of columns in
	// current matrix must be equel to number of rows in the provided matrix.
	template<MatrixElementId right_columns>
	Matrix<rows, right_columns, ElementType> Multiply(const Matrix<columns, right_columns, ElementType>& right) const
	{
		Matrix<rows, right_columns, ElementType> out;
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < right_columns; j++)
			{
				ElementType sum = static_cast<ElementType>(0);
				for (int k = 0; k < columns; k++)
				{
					sum += (*this)(i, k) * right(k, j);
				}
				out(i, j) = sum;
			}
		}
		return out;
	}

	// Returns a transposed matrix.
	Matrix<columns, rows, ElementType> Transpose() const
	{
		Matrix<columns, rows, ElementType> out;
		for (MatrixElementId i = 0; i < columns; i++)
		{
			for (MatrixElementId j = 0; j < rows; j++)
			{
				out(j, i) = (*this)(i, j);
			}
		}
		return out;
	}

	// Returns an inverted matrix.
	//    @return - optionally inverted matrix, if result has no value
	//              means that current matrix is degenerate.
	Optional<Matrix> Invert() const
	{
		static_assert(rows == columns, "Only square matrix can be inverted.");

		MatrixElementId P[rows];
		Optional<Matrix> lup_result = LUP(P);
		if (!lup_result)
		{
			// degenerate matrix
			return Optional<Matrix>();
		}

		Matrix LU = lup_result.Move();

		Matrix out;
		for (MatrixElementId j = 0; j < rows; j++)
		{
			for (MatrixElementId i = 0; i < rows; i++)
			{
				if (P[i] == j)
				{
					out(i, j) = static_cast<ElementType>(1);
				}
				else
				{
					out(i, j) = static_cast<ElementType>(0);
				}

				for (MatrixElementId k = 0; k < i; k++)
				{
					out(i, j) -= LU(i, k) * out(k, j);
				}
			}

			for (MatrixElementId i = rows; i-- > 0;)
			{
				for (MatrixElementId k = i + 1; k < rows; k++)
				{
					out(i, j) -= LU(i, k) * out(k, j);
				}

				out(i, j) = out(i, j) / LU(i, i);
			}
		}

		return out;
	}

	// Returns dot product.
	ElementType Dot(const Matrix& other) const
	{
		ElementType out = static_cast<ElementType>(0);
		for (MatrixElementId i = 0; i < size; i++)
		{
			out += table[i] * other.table[i];
		}
		return out;
	}

	// Returns cross product.
	Matrix Cross(const Matrix& other) const
	{
		static_assert(rows == 1 && columns == 3, "Cross product can be performed only for 3d vectors.");
		return Matrix(Y() * other.Z() - Z() * other.Y(),
		              Z() * other.X() - X() * other.Z(),
		              X() * other.Y() - Y() * other.X());
	}

	// Returns normalized vector
	Matrix Normalize() const
	{
		Matrix out;
		ElementType len = Length();
		for (MatrixElementId i = 0; i < size; i++)
		{
			out.table[i] /= len;
		}
		return out;
	}

	// Returns length of the vector.
	ElementType Length() const
	{
		return Sqrt(Dot(*this));
	}

	// Returns a pointer to the first element.
	const ElementType* GetData() const
	{
		return static_cast<const ElementType*>(table);
	}

	// Returns a constant reference to the named "X" element.
	const ElementType& X() const
	{
		static_assert(columns >= 1, "There is no element with such name.");
		return table[0];
	}

	// Returns a reference to the named "X" element.
	ElementType& X()
	{
		static_assert(columns >= 1, "There is no element with such name.");
		return table[0];
	}

	// Returns a constant reference to the named "Y" element.
	const ElementType& Y() const
	{
		static_assert(columns >= 2, "There is no element with such name.");
		return table[1];
	}

	// Returns a reference to the named "Y" element.
	ElementType& Y()
	{
		static_assert(columns >= 2, "There is no element with such name.");
		return table[1];
	}

	// Returns a constant reference to the named "Z" element.
	const ElementType& Z() const
	{
		static_assert(columns >= 3, "There is no element with such name.");
		return table[2];
	}

	// Returns a reference to the named "Z" element.
	ElementType& Z()
	{
		static_assert(columns >= 3, "There is no element with such name.");
		return table[2];
	}

	// Returns a constant reference to the named "W" element.
	const ElementType& W() const
	{
		static_assert(columns >= 4, "There is no element with such name.");
		return table[3];
	}

	// Returns a reference to the named "W" element.
	ElementType& W()
	{
		static_assert(columns >= 4, "There is no element with such name.");
		return table[3];
	}

private:
	// Swaps desired rows.
	void SwapRows(MatrixElementId row1, MatrixElementId row2)
	{
		for (int j = 0; j < columns; j++)
		{
			ElementType temp = (*this)(row1, j);
			(*this)(row1, j) = (*this)(row2, j);
			(*this)(row2, j) = temp;
		}
	}

	// LUP-Decomposition.
	Optional<Matrix> LUP(MatrixElementId P[rows]) const
	{
		MatrixElementId i;

		Matrix C = *this;

		for (i = 0; i < rows; i++)
		{
			P[i] = i;
		}

		for (i = 0; i < rows; i++)
		{
			ElementType pivot_abs = static_cast<ElementType>(0);
			Optional<MatrixElementId> pivot_row;

			for (MatrixElementId r = i; r < rows; r++)
			{
				ElementType abs_val = Abs(C(r, i));
				if (abs_val > pivot_abs)
				{
					pivot_abs = abs_val;
					pivot_row = r;
				}
			}

			if (pivot_abs < Precision<ElementType>::epsilon)
			{
				//degenerate matrix
				return Optional<Matrix>();
			}

			if (pivot_row && pivot_row.Get() != i)
			{
				MatrixElementId tmp = P[i];
				P[i] = P[pivot_row.Get()];
				P[pivot_row.Get()] = tmp;

				C.SwapRows(i, pivot_row.Get());
			}

			for (MatrixElementId j = i + 1; j < rows; j++)
			{
				C(j, i) /= C(i, i);

				for (MatrixElementId k = i + 1; k < rows; k++)
				{
					C(j, k) -= C(j, i) * C(i, k);
				}
			}
		}
		return C;
	}

	// Elements
	ElementType table[size];
};

// Vector is defined as a matrix with only one row.
template<MatrixElementId dim> using Vector = Matrix<1, dim>;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//