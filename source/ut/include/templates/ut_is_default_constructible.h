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
	// Old gcc versions give invalid value for this expression.
	// I can't figure out why. The only chance to cope with it -
	// to derive class from ut::NonDefaultConstructible manually.
	//
	// broken variant: sizeof((T)T())
	template<class T, void (T::*)()> struct Empty {};
	template<class C> compile_time::no Check(Empty<C, &C::MarkAsNonConstructible>*);
	template<class C> compile_time::yes Check(...);
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