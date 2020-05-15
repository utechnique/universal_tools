//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "math/ut_precision.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// 'Pi' constant;
static const float skPi = Precision<float>::pi;

//----------------------------------------------------------------------------//
// Computes the sine of the given value.
template<typename T>
T Sin(T x)
{
	return static_cast<T>(sinf(static_cast<float>(x)));
}

// Computes the arc sine of the given value.
template<typename T>
T ArcSin(T x)
{
	return static_cast<T>(asinf(static_cast<float>(x)));
}

// Sine specialization for 'double' type.
template<> double Sin<double>(const double);
template<> double ArcSin<double>(const double);

//----------------------------------------------------------------------------//
// Computes the cosine of the given value.
template<typename T>
T Cos(T x)
{
	return static_cast<T>(cosf(static_cast<float>(x)));
}

// Computes the arc cosine of the given value.
template<typename T>
T ArcCos(T x)
{
	return static_cast<T>(acosf(static_cast<float>(x)));
}

// Cosine specialization for 'double' type.
template<> double Cos<double>(double);
template<> double ArcCos<double>(double);

//----------------------------------------------------------------------------//
// Computes the tangent of the given value.
template<typename T>
T Tan(T x)
{
	return static_cast<T>(tanf(static_cast<float>(x)));
}

// Computes the arc tangent of the given value.
template<typename T>
T ArcTan(T x)
{
	return static_cast<T>(atanf(static_cast<float>(x)));
}

// Tangent specialization for 'double' type.
template<> double Tan<double>(double);
template<> double ArcTan<double>(double);

//----------------------------------------------------------------------------//
// Computes the arc tangent of @y/@x using the signs of arguments to determine
// the correct quadrant.
template<typename T>
T ArcTan2(T y, T x)
{
	return static_cast<T>(atan2f(static_cast<float>(y), static_cast<float>(x)));
}

// Arc tangent 2 specialization for 'double' type.
template<> double ArcTan2<double>(double, double);

//----------------------------------------------------------------------------//
// Computes the cotangent of the given value.
template<typename T>
T Cot(T x)
{
	return Cos(x) / Sin(x);
}

//----------------------------------------------------------------------------//
// Converts radians to degrees.
template<typename T>
T ToDegrees(T radians)
{
	return radians * static_cast<T>(180) / Precision<T>::pi;
}

//----------------------------------------------------------------------------//
// Converts degrees to radians.
template<typename T>
T ToRadiands(T degrees)
{
	return degrees * Precision<T>::pi / static_cast<T>(180);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//