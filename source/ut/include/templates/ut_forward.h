//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_def.h"
#include "common/ut_platform.h"
#include "templates/ut_remove_ref.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Casts a reference to an lvalue reference.
template <typename T>
FORCEINLINE constexpr T&& Forward(typename RemoveReference<T>::Type& ref)
{
	return static_cast<T&&>(ref);
}

// Casts a reference to an rvalue reference.
template <typename T>
FORCEINLINE constexpr T&& Forward(typename RemoveReference<T>::Type&& ref)
{
	return static_cast<T&&>(ref);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//