//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Precision traits for numeric types.
template<typename T> struct Precision { static constexpr T epsilon = 0; };

// Precision traits for 'float' type.
template<> struct Precision<float> { static constexpr float epsilon = 1e-6f; };

// Precision traits for 'double' type.
template<> struct Precision<double> { static constexpr double epsilon = 1e-9; };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//