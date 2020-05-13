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
T Sqrt(const T& a)
{
	return static_cast<T>(sqrtf(static_cast<float>(a)));
}

// Square root specialization for 'double' type.
template<> double Sqrt<double>(const double& a);

//----------------------------------------------------------------------------//
// Computes the value of @base raised to the power @exp.
template<typename T>
T Pow(const T& base, const T& exp)
{
	return static_cast<T>(powf(static_cast<float>(base), static_cast<float>(exp)));
}

// Power specialization for 'double' type.
template<> double Pow(const double& base, const double& exp);

//----------------------------------------------------------------------------//
// Computes the natural logarithm of the given value.
template<typename T>
T Logarithm(const T& a)
{
	return static_cast<T>(logf(static_cast<float>(a)));
}

// Logarithm specialization for 'double' type.
template<> double Logarithm<double>(const double& a);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//