//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "ut_pointer_traits.h"
#include "preprocessor/ut_enum.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// This struct has specializations according to the provided return type and
// number of arguments (arity) of the function signature.
template<typename... Signature> struct FunctionTraitsHelper;

// Helper function to get type of the argument in the function signature
// knowing it's index `arg_id`
namespace function_traits_helper
{
	template<int arg_id, typename Argument, typename... Tail>
	struct ArgumentExtractor
	{
		typedef typename ArgumentExtractor<arg_id - 1, Tail...>::Type Type;
	};

	template<typename Argument, typename... Tail>
	struct ArgumentExtractor<0, Argument, Tail...>
	{
		typedef Argument Type;
	};
}

// Specialization of the ut::FunctionTraitsHelper template, where
// signature of the function is divided into return-type and variadic
// list of arguments.
template<typename R, typename... Args>
struct FunctionTraitsHelper<R(*)(Args...)>
{
	typedef R ReturnType;

	template<int arg_id>
	using Argument = function_traits_helper::ArgumentExtractor<arg_id, Args...>;
};

// Specialization for the case when argument list is empty.
template<typename R>
struct FunctionTraitsHelper<R(*)(void)>
{
	typedef R ReturnType;
};

//----------------------------------------------------------------------------//
// Use ut::FunctionTraits to define number of arguments (arity), return type
// and type of the every argument of the provided function signature.
// Example:
// FunctionTraits<int(float,bool)>::ReturnType r;           // int
// FunctionTraits<int(float,bool)>::Argument<0>::Type arg1; // float
// FunctionTraits<int(float,bool)>::Argument<1>::Type arg2; // bool
template<typename Function>
struct FunctionTraits : public FunctionTraitsHelper<typename AddPointer<Function>::Type>
{ };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//