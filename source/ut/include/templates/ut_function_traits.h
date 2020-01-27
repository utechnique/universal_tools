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
#if CPP_STANDARD >= 2011
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
#else //CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// This implementation is inspired by function_traits template in boost library.

//----------------------------------------------------------------------------//
// Defines the maximum number of function arguments, when it's still possible
// to get it's traits (such as return type, arguments type and arity) at compile
// time. In other words - it's maximum arity of the ut::FunctionTraits<>.
#define UT_FUNCTION_MAX_ARITY 10

// 1: , Tj
#define UT_FUNCTION_TRAITS_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) T##id
// 2: , typename Tj
#define UT_FUNCTION_TRAITS_FULL_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename T##id 
// 3: , typename Tj = void
#define UT_FUNCTION_TRAITS_VOID_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename T##id = void
// 4: Argument extractor
#define UT_FUNCTION_TRAITS_EXTRACTOR_SPECIALIZATION(id)                                          \
    template < UT_PP_ENUM_IN(UT_FUNCTION_MAX_ARITY, UT_FUNCTION_TRAITS_FULL_TYPENAME_ITERATOR) > \
    struct ArgumentExtractor< id,                                                                \
        UT_PP_ENUM_IN(UT_FUNCTION_MAX_ARITY, UT_FUNCTION_TRAITS_TYPENAME_ITERATOR)               \
    >                                                                                            \
    {                                                                                            \
        typedef T##id Type;                                                                      \
    };

//----------------------------------------------------------------------------//
// Helper structure to extract argument type from the signature by it's id.
namespace function_traits_helper
{
	template<int arg_id, UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_FUNCTION_TRAITS_VOID_TYPENAME_ITERATOR)>
	struct ArgumentExtractor { typedef void* ArgType; };

	UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_FUNCTION_TRAITS_EXTRACTOR_SPECIALIZATION)
}

//----------------------------------------------------------------------------//
// This struct has specializations according to the provided return type and
// number of arguments (arity) of the function signature.
template<typename Function> struct FunctionTraitsHelper;

//----------------------------------------------------------------------------//
// Macro for declaring specialized versions of the ut::FunctionTraitsHelper
// template.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename R, typename T0, typename T1, ... typename Tj >
// struct FunctionTraitsHelper<R(*)(T0, T1, ... Tj)>
// {
//     typedef R ReturnType;
//     template <int arg_id> struct Argument
//     {
//         typedef typename function_traits_helper::ArgumentExtractor<j, T0, ... Tj>::Type Type;
//     }
// };
#define UT_FUNCTION_TRAIT_SPECIALIZATION(id)                                                   \
template <                                                                                     \
    typename R UT_PP_COMMA_IF(id) UT_PP_ENUM_IN(id, UT_FUNCTION_TRAITS_FULL_TYPENAME_ITERATOR) \
>                                                                                              \
struct FunctionTraitsHelper<R(*)( UT_PP_ENUM_IN(id, UT_FUNCTION_TRAITS_TYPENAME_ITERATOR) )>   \
{                                                                                              \
    typedef R ReturnType;                                                                      \
    template <int arg_id> struct Argument                                                      \
    {                                                                                          \
        typedef typename function_traits_helper::ArgumentExtractor<                            \
            arg_id UT_PP_COMMA_IF(id) UT_PP_ENUM_IN(id, UT_FUNCTION_TRAITS_TYPENAME_ITERATOR)  \
        >::Type Type;                                                                          \
    };                                                                                         \
};

// All specialized variations are declared here.
UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_FUNCTION_TRAIT_SPECIALIZATION)

//----------------------------------------------------------------------------//
#endif //CPP_STANDARD >= 2011
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