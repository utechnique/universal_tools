//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "math/ut_matrix.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Quaternion. Precision can be set via template argument.
template<typename Scalar = float>
class Quaternion
{
public:
	// members
	Scalar r, i, j, k;

	// Creates a new quaternion from angle and axis vector.
	//    @param angle - an angle in degrees of rotation around the @axis.
	//    @param axis - 3d vector representing rotation axis.
	//    @return - a new quaternion.
	static Quaternion MakeFromAngleAndAxis(Scalar angle, const Vector<3, Scalar>& axis)
	{
		Quaternion q;

		q.i = axis.X();
		q.j = axis.Y();
		q.k = axis.Z();

		const Scalar i_length = static_cast<Scalar>(1) / Sqrt(q.i*q.i + q.j*q.j + q.k*q.k);

		q.i = q.i * i_length;
		q.j = q.j * i_length;
		q.k = q.k * i_length;

		Scalar half = Precision<Scalar>::pi * angle / static_cast<Scalar>(360);

		q.r = Cos(half);

		const Scalar sin_theta_over_two = Sin(half);
		q.i = q.i * sin_theta_over_two;
		q.j = q.j * sin_theta_over_two;
		q.k = q.k * sin_theta_over_two;

		return q;
	}

	// Creates a new quaternion from 3 angles.
	//    @param angles - 3d vector, each element represents an angle
	//                    in degrees of rotation around the corresponding
	//                    axis.
	//    @return - a new quaternion.
	static Quaternion MakeFromAngles(const Vector<3, Scalar>& angles)
	{
		Quaternion q;

		const Scalar yaw = ToRadiands(angles.X());
		const Scalar pitch = ToRadiands(angles.Y());
		const Scalar roll = ToRadiands(angles.Z());

		const Scalar two = static_cast<Scalar>(2);

		q.i = Sin(yaw / two) * Cos(pitch / two) * Sin(roll / two) + Cos(yaw / two) * Sin(pitch / two) * Cos(roll / two);
		q.j = Sin(yaw / two) * Cos(pitch / two) * Cos(roll / two) - Cos(yaw / two) * Sin(pitch / two) * Sin(roll / two);
		q.k = Cos(yaw / two) * Cos(pitch / two) * Sin(roll / two) - Sin(yaw / two) * Sin(pitch / two) * Cos(roll / two);
		q.r = Cos(yaw / two) * Cos(pitch / two) * Cos(roll / two) + Sin(yaw / two) * Sin(pitch / two) * Sin(roll / two);

		return q;
	}

	// Creates a new quaternion representing a rotation
	// from one axis to another.
	//    @param v0 - first axis (must be normalized).
	//    @param v1 - second axis (must be normalized).
	//    @return - a new quaternion.
	static Quaternion MakeShortestRotation(const Vector<3, Scalar>& v0,
	                                       const Vector<3, Scalar>& v1) 
	{
		Vector<3, Scalar> c = v0.Cross(v1);
		const Scalar d = v0.Dot(v1);

		const Scalar zero = static_cast<Scalar>(0);
		const Scalar one = static_cast<Scalar>(1);
		const Scalar two = static_cast<Scalar>(2);

		if (d < -one + Precision<Scalar>::epsilon)
		{
			Vector<3, Scalar> n;

			if (Abs(v0.Z()) > Precision<Scalar>::epsilon)
			{
				const float a = v0.Y() * v0.Y() + v0.Z() * v0.Z();
				const float k = one / Sqrt(a);
				n.X() = zero;
				n.Y() = -v0.Z() * k;
				n.Z() = v0.Y() * k;
			}
			else
			{
				const float a = v0.X() * v0.X() + v0.Y() * v0.Y();
				const float k = one / Sqrt(a);
				n.X() = -v0.Z() * k;
				n.Y() = v0.X() * k;
				n.Z() = zero;
			}

			return Quaternion(zero, n.X(), n.Y(), n.Z());
		}

		const Scalar s = Sqrt((one + d) * two);
		const Scalar rs = one / s;

		return Quaternion(s / two, c.X() * rs, c.Y() * rs, c.Z() * rs);
	}

	// Constructor.
	Quaternion() : r(static_cast<Scalar>(1))
	             , i(static_cast<Scalar>(0))
	             , j(static_cast<Scalar>(0))
	             , k(static_cast<Scalar>(0))
	{}

	// Constructor.
	Quaternion(Scalar in_r,
	           Scalar in_i,
	           Scalar in_j,
	           Scalar in_k) : r(in_r)
	                        , i(in_i)
	                        , j(in_j)
	                        , k(in_k)
	{}

	// Comparison operator, each element is compared one by one.
	//    @param other - quaternion to compare with.
	bool operator == (const Quaternion& other) const
	{
		return Equal(r, other.r) && Equal(i, other.i) &&
		       Equal(j, other.j) && Equal(k, other.k);
	}

	// Negation comparison operator, each element is compared one by one.
	//    @param other - quaternion to compare with.
	bool operator != (const Quaternion& other) const
	{
		return !this->operator == (other);
	}

	// Multiplication operator, each element is
	// multiplied by provided scalar value.
	Quaternion operator * (Scalar s) const
	{
		return Quaternion(r * s, i * s, j * s, k * s);
	}

	// Multiplication operator.
	Quaternion operator * (const Quaternion& right) const
	{
		Quaternion q = *this;
		Quaternion result;
		result.r = q.r*right.r - q.i*right.i - q.j*right.j - q.k*right.k;
		result.i = q.r*right.i + q.i*right.r + q.j*right.k - q.k*right.j;
		result.j = q.r*right.j + q.j*right.r + q.k*right.i - q.i*right.k;
		result.k = q.r*right.k + q.k*right.r + q.i*right.j - q.j*right.i;
		return result;
	}

	// Multiplication operator, accepts 3d vector.
	Quaternion operator * (const Vector<3, Scalar>& right) const
	{
		Scalar a, b, c, d;

		a = -i*right.X() - j*right.Y() - k *right.Z();
		b = r*right.X() + j*right.Z() - right.Y()*k;
		c = r*right.Y() + k*right.X() - right.Z()*i;
		d = r*right.Z() + i*right.Y() - right.X()*j;

		return Quaternion(a, b, c, d);
	}

	// Multiplication assignment operator.
	void operator *= (const Quaternion& right)
	{
		Quaternion q = *this;
		r = q.r*right.r - q.i*right.i -
		    q.j*right.j - q.k*right.k;
		i = q.r*right.i + q.i*right.r +
		    q.j*right.k - q.k*right.j;
		j = q.r*right.j + q.j*right.r +
		    q.k*right.i - q.i*right.k;
		k = q.r*right.k + q.k*right.r +
		    q.i*right.j - q.j*right.i;
	}

	// Dot product.
	Scalar Dot(const Quaternion& q)  const
	{
		return i*q.i + j*q.j + k*q.k + r*q.r;
	}

	// Returns normalized quaternion.
	Quaternion Normalize() const
	{
		Quaternion q = *this;

		Scalar d = q.r*q.r + q.i*q.i + q.j*q.j + q.k*q.k;

		// check for zero length ut::Quaternion, and use the no-rotation
		// ut::Quaternion in that case.
		if (d == 0)
		{
			q.r = 1;
			return q;
		}

		d = 1.0f / Sqrt(d);
		q.r *= d;
		q.i *= d;
		q.j *= d;
		q.k *= d;

		return q;
	}

	// Returns inverted quaternion.
	Quaternion Invert() const
	{
		return Quaternion(r, -i, -j, -k);
	}

	// Rotates provided vector and returns the result.
	Vector<3, Scalar> Rotate(const Vector<3, Scalar>& v) const
	{
		Quaternion inverted;
		inverted.i = -i;
		inverted.j = -j;
		inverted.k = -k;
		inverted.r = r;

		Quaternion left = (*this) * v;

		Vector<3, Scalar> out;
		out.X() = left.r*inverted.i + inverted.r*left.i + left.j*inverted.k - inverted.j*left.k;
		out.Y() = left.r*inverted.j + inverted.r*left.j + left.k*inverted.i - inverted.k*left.i;
		out.Z() = left.r*inverted.k + inverted.r*left.k + left.i*inverted.j - inverted.i*left.j;

		return out;
	}

	// Performes spherical linear interpolation between this quaternion and the given one.
	//    @param q - quaternion to interpolate with.
	//    @param t - interpolation parameter (between 0 and 1)
	//    @return - interpolated quaternion.
	Quaternion Slerp(const Quaternion& q, Scalar t) const
	{
		const Scalar zero = static_cast<Scalar>(0);
		const Scalar one = static_cast<Scalar>(1);

		Scalar len = Sqrt(Length2() * q.Length2());

		Scalar prod = Dot(q) / len;

		if (prod > one) prod = one;

		if (prod < -one) prod = -one;

		if (!Equal(Abs(prod), one))
		{
			const Scalar sign = (prod < zero) ? -one : one;
			const Scalar theta = ArcCos(sign * prod);
			const Scalar s1 = Sin(sign * t * theta);
			const Scalar d = one / Sin(theta);
			const Scalar s0 = Sin((one - t)*theta);

			return Quaternion( (r*s0 + q.r*s1) * d,
			                   (i*s0 + q.i*s1) * d,
			                   (j*s0 + q.j*s1) * d,
			                   (k*s0 + q.k*s1) * d);
		}
		else
		{
			return *this;
		}
	}	

	// Converts this quaternion to 3d vector, where each element represents
	// an angle (in degrees).
	Vector<3, Scalar> ToAngles() const
	{
		Vector<3, Scalar> out;

		Scalar x = i;
		Scalar y = j;
		Scalar z = k;
		Scalar w = r;

		Scalar sqw = w * w;
		Scalar sqx = x * x;
		Scalar sqy = y * y;
		Scalar sqz = z * z;

		// constants
		const Scalar one = static_cast<Scalar>(1);
		const Scalar two = static_cast<Scalar>(2);

		// heading = rotation about z-axis
		out.Z() = ArcTan2(two * (x * y + z * w), (sqx - sqy - sqz + sqw));
		// bank = rotation about x-axis
		out.X() = ArcTan2(two * (y * z + x * w), (-sqx - sqy + sqz + sqw));
		// attitude = rotation about y-axis
		out.Y() = ArcSin(-two * (x * z - y * w));

		out.X() = ArcTan2((two*(w*y + x*z)), (one - two*(y*y + x*x)));

		out.Y() = ArcSin(two*(w*x - z*y));

		out.Z() = ArcTan2((two*(w*z + y*x)), (one - two*(x*x + z*z)));

		return out;
	}

	// Converts this quaternion to transform (3x3) matrix.
	template<MatrixElementId matrix_size>
	Matrix<matrix_size, matrix_size, Scalar> ToTransform() const
	{
		const Scalar one = static_cast<Scalar>(1);
		const Scalar two = static_cast<Scalar>(2);

		Matrix<matrix_size, matrix_size, Scalar> matrix;

		matrix(0, 0) = one - j*j*two - k*k*two;
		matrix(0, 1) = i*j*two - r*k*two;
		matrix(0, 2) = i*k*two + r*j*two;

		matrix(1, 0) = i*j*two + r*k*two;
		matrix(1, 1) = one - i*i*two - k*k*two;
		matrix(1, 2) = j*k*two - r*i*two;

		matrix(2, 0) = i*k*two - r*j*two;
		matrix(2, 1) = j*k*two + r*i*two;
		matrix(2, 2) = one - i*i*two - j*j*two;

		return matrix;
	}

	// Returns square of length.
	Scalar Length2() const
	{
		return r*r + i*i + j*j + k*k;
	}

	// Returns angle in degrees.
	Scalar GetAngle() const
	{
		Scalar s = static_cast<Scalar>(2) * ToDegrees(ArcCos(r));
		return s;
	}

	// Returns 3d axis vector.
	Vector<3, Scalar> GetAxis() const
	{
		return Vector<3, Scalar>(i, j, k);
	}	
};

// Specialized type name function for quaternion
template <typename Scalar>
struct Type< Quaternion<Scalar> >
{
	static inline const char* Name() { return "quaternion"; }
};

// Shorter quaternion name versions
using Quat = Quaternion<float>;
using Quat4f = Quaternion<float>;
using Quat4d = Quaternion<double>;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//