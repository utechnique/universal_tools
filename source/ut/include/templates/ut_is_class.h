//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "ut_integral_constant.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
namespace is_class_trait
{
	// if @T is a class, this variant will be called,
	// and the size of the returned value would be 1 byte
	template <class T> compile_time::yes Check(int T::*);

	// if @T is not a class, this variant will be called,
	// and the size of the returned value would be 2 bytes
	template <class T> compile_time::no Check(...);
}

// Use ut::IsClass<class T> to define at compile time if @T is a class type.
template <class T>
struct IsClass : IntegralConstant
	<
	bool,
	sizeof(is_class_trait::Check<T>(0)) == sizeof(compile_time::yes)
	> {};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//