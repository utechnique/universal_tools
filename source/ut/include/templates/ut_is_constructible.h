//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Enable this macro if your compiler supports '__is_constructible'
// intrinsic function.
#define UT_INTRINSIC_IS_CONSTRUCTIBLE CPP_STANDARD >= 2011 && UT_WINDOWS

//----------------------------------------------------------------------------//
// Check if class is constructible with provided arguments
#if UT_INTRINSIC_IS_CONSTRUCTIBLE
template <typename T, typename... Args>
struct IsConstructible
{
	enum { value = __is_constructible(T, Args...) };
};
#endif

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//