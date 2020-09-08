//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "math/ut_precision.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Computes the square root of the given value.
template<typename T>
T Sqrt(T a)
{
	return static_cast<T>(sqrtf(static_cast<float>(a)));
}

// Square root specialization for 'double' type.
template<> double Sqrt<double>(double);

//----------------------------------------------------------------------------//
// Computes the value of @base raised to the power @exp.
template<typename T>
T Pow(T base, T exp)
{
	return static_cast<T>(powf(static_cast<float>(base), static_cast<float>(exp)));
}

// Power specialization for 'double' type.
template<> double Pow(double, double);

//----------------------------------------------------------------------------//
// Computes the natural logarithm of the given value.
template<typename T>
T Logarithm(T a)
{
	return static_cast<T>(logf(static_cast<float>(a)));
}

// Logarithm specialization for 'double' type.
template<> double Logarithm<double>(double a);

//----------------------------------------------------------------------------//
// Computes the binary (base-2) logarithm.
template<typename T>
T Logarithm2(T a)
{
	return static_cast<T>(Logarithm<T>(a) / 0.693147180559945309417);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//