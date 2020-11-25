//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
#define UT_INTRINSIC_IS_CONSTRUCTIBLE 1

// Check if class is constructible with provided arguments
template<typename T, typename... Args>
struct IsConstructible
{
#if UT_INTRINSIC_IS_CONSTRUCTIBLE
	enum { value = __is_constructible(T, Args...) };
#endif
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
