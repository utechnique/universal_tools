//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "math/ut_cmp.h"
#include "math/ut_pow.h"
#include "templates/ut_more_than.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Type of element's index inside a matrix.
typedef uint32 MatrixElementId;

// Matrix tags.
struct MatrixGeneralTag {};
struct MatrixVertorTag {};
struct MatrixColorTag {};

// ut::Matrix is a complete template class for matrices with custom static size.
// It can represent both matrix and vector (as a matrix with only one row).
template<MatrixElementId rows,
         MatrixElementId columns,
         typename Scalar = float,
         typename Tag = MatrixGeneralTag>
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
		Matrix out(static_cast<Scalar>(0));
		for (MatrixElementId i = 0; i < columns; i++)
		{
			out(i, i) = static_cast<Scalar>(1);
		}
		return out;
	}

	// Constructor. Elements are not initialized.
	Matrix()
	{}

	// Constructor. All elements are initialized with a scalar value.
	explicit Matrix(Scalar scalar)
	{
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = scalar;
		}
	}

	// Constructor. Initializes elements from the provided arguments.
	// Number of arguments must be exactly the same as number of elements in this matrix.
	// Each argument must be convertible to @Scalar.
	template<typename... Elements, typename Sfinae = typename EnableIf<MoreThan<1, Elements...>::value>::Type>
	Matrix(Elements... elements)
	{
		// convert all arguments to the desired type
		Scalar args[]{ static_cast<Scalar>(elements)... };

		// check if number of provided arguments is exactly
		// the same as number of elements in the matrix
		static_assert(sizeof(args) / sizeof(Scalar) == size, "Invalid number of arguments");
		
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

	// Copy constructor, accepts another tag.
	template<typename OtherTag>
	Matrix(const Matrix<rows, columns, Scalar, OtherTag>& copy)
	{
		const Scalar* copy_data = copy.GetData();
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = copy_data[i];
		}
	}

	// Assignment operator, all elements are set to the provided scalar value.
	Matrix& operator = (Scalar scalar)
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

	// Assignment operator, accepts another tag.
	template<typename OtherTag>
	Matrix& operator = (const Matrix<rows, columns, Scalar, OtherTag>& copy)
	{
		const Scalar* copy_data = copy.GetData();
		for (MatrixElementId i = 0; i < size; i++)
		{
			table[i] = copy_data[i];
		}
		return *this;
	}

	// Returns a constant reference to the element that is defined by it's row and column.
	//    @param row - index of the row desired element belongs to.
	//    @param column - index of the column desired element belongs to.
	//    @return - constant reference to the desired element.
	const Scalar& operator()(MatrixElementId row, MatrixElementId column) const
	{
		UT_ASSERT(row * column < size);
		return table[row*columns + column];
	}

	// Returns a reference to the element that is defined by it's row and column.
	//    @param row - index of the row desired element belongs to.
	//    @param column - index of the column desired element belongs to.
	//    @return - reference to the desired element.
	Scalar& operator()(MatrixElementId row, MatrixElementId column)
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
		Matrix<rows, columns, Scalar> out;
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
	Matrix operator * (const Matrix<columns, columns, Scalar>& right) const
	{
		return Multiply<columns>(right);
	}

	// Multiplies current matrix with the provided one. Number of columns in
	// current matrix must be equel to number of rows in the provided matrix.
	template<MatrixElementId right_columns>
	Matrix<rows, right_columns, Scalar> Multiply(const Matrix<columns, right_columns, Scalar>& right) const
	{
		Matrix<rows, right_columns, Scalar> out;
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < right_columns; j++)
			{
				Scalar sum = static_cast<Scalar>(0);
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
	Matrix<columns, rows, Scalar> Transpose() const
	{
		Matrix<columns, rows, Scalar> out;
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
					out(i, j) = static_cast<Scalar>(1);
				}
				else
				{
					out(i, j) = static_cast<Scalar>(0);
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
	Scalar Dot(const Matrix& other) const
	{
		Scalar out = static_cast<Scalar>(0);
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
		Matrix out(*this);
		Scalar len = Length();
		for (MatrixElementId i = 0; i < size; i++)
		{
			out.table[i] /= len;
		}
		return out;
	}

	// Returns length of the vector.
	Scalar Length() const
	{
		return Sqrt(Dot(*this));
	}

	// Returns a const pointer to the first element.
	const Scalar* GetData() const
	{
		return static_cast<const Scalar*>(table);
	}

	// Returns a pointer to the first element.
	Scalar* GetData()
	{
		return static_cast<Scalar*>(table);
	}

	// Returns a constant reference to the named "X" element.
	const Scalar& X() const
	{
		static_assert(columns >= 1, "There is no element with such name.");
		return table[0];
	}

	// Returns a reference to the named "X" element.
	Scalar& X()
	{
		static_assert(columns >= 1, "There is no element with such name.");
		return table[0];
	}

	// Returns a constant reference to the named "R" element.
	const Scalar& R() const
	{
		return X();
	}

	// Returns a reference to the named "R" element.
	Scalar& R()
	{
		return X();
	}

	// Returns a constant reference to the named "Y" element.
	const Scalar& Y() const
	{
		static_assert(columns >= 2, "There is no element with such name.");
		return table[1];
	}

	// Returns a reference to the named "Y" element.
	Scalar& Y()
	{
		static_assert(columns >= 2, "There is no element with such name.");
		return table[1];
	}

	// Returns a constant reference to the named "G" element.
	const Scalar& G() const
	{
		return Y();
	}

	// Returns a reference to the named "G" element.
	Scalar& G()
	{
		return Y();
	}

	// Returns a constant reference to the named "Z" element.
	const Scalar& Z() const
	{
		static_assert(columns >= 3, "There is no element with such name.");
		return table[2];
	}

	// Returns a reference to the named "Z" element.
	Scalar& Z()
	{
		static_assert(columns >= 3, "There is no element with such name.");
		return table[2];
	}

	// Returns a constant reference to the named "B" element.
	const Scalar& B() const
	{
		return Z();
	}

	// Returns a reference to the named "B" element.
	Scalar& B()
	{
		return Z();
	}

	// Returns a constant reference to the named "W" element.
	const Scalar& W() const
	{
		static_assert(columns >= 4, "There is no element with such name.");
		return table[3];
	}

	// Returns a reference to the named "W" element.
	Scalar& W()
	{
		static_assert(columns >= 4, "There is no element with such name.");
		return table[3];
	}

	// Returns a constant reference to the named "A" element.
	const Scalar& A() const
	{
		return W();
	}

	// Returns a reference to the named "A" element.
	Scalar& A()
	{
		return W();
	}

private:
	// Swaps desired rows.
	void SwapRows(MatrixElementId row1, MatrixElementId row2)
	{
		for (int j = 0; j < columns; j++)
		{
			Scalar temp = (*this)(row1, j);
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
			Scalar pivot_abs = static_cast<Scalar>(0);
			Optional<MatrixElementId> pivot_row;

			for (MatrixElementId r = i; r < rows; r++)
			{
				Scalar abs_val = Abs(C(r, i));
				if (abs_val > pivot_abs)
				{
					pivot_abs = abs_val;
					pivot_row = r;
				}
			}

			if (pivot_abs < Precision<Scalar>::epsilon)
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
	Scalar table[size];
};

//----------------------------------------------------------------------------//
// Vector is defined as a matrix with only one row.
template<MatrixElementId dim, typename Scalar = float>
using Vector = Matrix<1, dim, Scalar, MatrixVertorTag>;

// Color is a vector too.
template<MatrixElementId dim, typename Scalar = float>
using Color = Matrix<1, dim, Scalar, MatrixColorTag>;

//----------------------------------------------------------------------------//
// Specialized type name function for matrices
template<MatrixElementId rows, MatrixElementId columns, typename Scalar>
struct Type< Matrix<rows, columns, Scalar, MatrixGeneralTag> >
{
	static const String skName;
	static inline const char* Name() { return skName.GetAddress(); }
};

template<MatrixElementId rows, MatrixElementId columns, typename Scalar>
const String Type< Matrix<rows, columns, Scalar> >::skName = String("matrix") + Print(rows) + "x" + Print(columns);

// Specialized type name function for vectors
template<MatrixElementId dim, typename Scalar>
struct Type< Vector<dim, Scalar> >
{
	static const String skName;
	static inline const char* Name() { return skName.GetAddress(); }
};

template<MatrixElementId dim, typename Scalar>
const String Type< Vector<dim, Scalar> >::skName = String("vector") + Print(dim);

// Specialized type name function for colors
template<MatrixElementId dim, typename Scalar>
struct Type< Color<dim, Scalar> >
{
	static const String skName;
	static inline const char* Name() { return skName.GetAddress(); }
};

template<MatrixElementId dim, typename Scalar>
const String Type< Color<dim, Scalar> >::skName = String("color") + Print(dim);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//