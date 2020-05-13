//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "math/ut_precision.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Returns the smaller of the given values.
template<typename T>
constexpr T Min(const T& first, const T& second)
{
	return first < second ? first : second;
}

// Returns the greater of the given values.
template<typename T>
constexpr T Max(const T& first, const T& second)
{
	return first > second ? first : second;
}

// Computes the absolute value of the given number.
template<typename T>
constexpr T Abs(const T& a)
{
	return a > static_cast<T>(0) ? a : -a;
}

// Returns true if two values are considered equal.
template<typename T>
bool Equal(const T& first, const T& second)
{
	return Abs(first - second) <= Precision<T>::epsilon;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//