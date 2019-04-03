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
// Pure interface traits
namespace abstract_trait
{
	// if @T is not an abstract class, this variant will be called,
	// and the size of the returned value would be 2 bytes
	template <class T> compile_time::no Check(T(*)[1]);

	// if @T is an abstract class, this variant will be called,
	// and the size of the returned value would be 1 byte
	template <class T> compile_time::yes Check(...);
}

// Use ut::IsAbstract<class T> to define at compile time if @T is an abstract type.
template <class T>
struct IsAbstract : IntegralConstant
	<
	bool,
	sizeof(abstract_trait::Check<T>(0)) == sizeof(compile_time::yes)
	> {};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//