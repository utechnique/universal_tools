//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "ut_is_constructible.h"
#include "ut_integral_constant.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Check if class is default constructible
namespace default_constructible_trait
{
	template<typename _Tp, typename = decltype(_Tp())>
	static compile_time::yes Check(int);

	template<typename>
	static compile_time::no Check(...);
}

template <class T>
struct IsDefaultConstructible : IntegralConstant
	<
	bool,
#if UT_INTRINSIC_IS_CONSTRUCTIBLE
	IsConstructible<T>::value
#else
	sizeof(default_constructible_trait::Check<T>(0)) == sizeof(compile_time::yes)
#endif
	> {};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//