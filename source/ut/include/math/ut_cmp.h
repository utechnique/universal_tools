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
constexpr T Min(T first, T second)
{
	return first < second ? first : second;
}

// Returns the greater of the given values.
template<typename T>
constexpr T Max(T first, T second)
{
	return first > second ? first : second;
}

// Clamps value.
template<typename T>
constexpr T Clamp(T value, T min, T max)
{
	return Min(Max(value, min), max);
}

// Computes the absolute value of the given number.
template<typename T>
constexpr T Abs(T a)
{
	return a > static_cast<T>(0) ? a : -a;
}

// Returns true if two values are considered equal.
template<typename T>
bool Equal(T first, T second)
{
	return Abs(first - second) <= Precision<T>::epsilon;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//