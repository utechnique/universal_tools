//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "ut_is_constructible.h"
#include "ut_integral_constant.h"
#include "ut_enable_if.h"
#include "ut_is_base_of.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Check if class is copy constructible
namespace copy_constructible_trait
{
	// Visual Studio compilers for c++11 and higher have a problem when a base
	// class (such as ut::NonCopyable) has a deleted copy constructor. In this
	// case the compiler thinks there really is a copy-constructor and tries to
	// instantiate the deleted member. std::is_copy_constructible has the same
	// issue (or at least returns an incorrect value, which just defers the
	// issue into the users code) as well. We can at least fix ut::NonCopyable
	// as a base class as a special case.
	//
	// broken variant: sizeof(T(*static_cast<T*>(0)))
	template<typename T> compile_time::yes Check(
		typename EnableIf<
			!IsBaseOf<NonCopyable, T>::value
		>::Type*
	);
	template<typename T> compile_time::no Check(...);
}

template <class T>
struct IsCopyConstructible : IntegralConstant
	<
	bool,
#if UT_INTRINSIC_IS_CONSTRUCTIBLE
	IsConstructible<T, const T&>::value
#else
	sizeof(copy_constructible_trait::Check<T>(0)) == sizeof(compile_time::yes)
#endif
	> {};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//