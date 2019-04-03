//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "ut_pointer_traits.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// This implementation is inspired by function_traits template in boost library.

//----------------------------------------------------------------------------//
// Defines the maximum number of function arguments, when it's still possible
// to get it's traits (such as return type, arguments type and arity) at compile
// time. In other words - it's maximum arity of the ut::FunctionTraits<>.
#define UT_FUNCTION_TRAITS_MAX_ARITY 10

//----------------------------------------------------------------------------//
// This struct has specializations according to the provided return type and
// number of arguments (arity) of the function signature.
template<typename Function> struct FunctionTraitsHelper;

//----------------------------------------------------------------------------//
// No arguments.
template<typename R>
struct FunctionTraitsHelper<R(*)(void)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 0);
	typedef R ReturnType;
};

//----------------------------------------------------------------------------//
// 1 argument.
template<typename R, typename T1>
struct FunctionTraitsHelper<R(*)(T1)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 1);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T1 argument_type;
};

//----------------------------------------------------------------------------//
// 2 arguments.
template<typename R, typename T1, typename T2>
struct FunctionTraitsHelper<R(*)(T1, T2)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 2);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
};

//----------------------------------------------------------------------------//
// 3 arguments.
template<typename R, typename T1, typename T2, typename T3>
struct FunctionTraitsHelper<R(*)(T1, T2, T3)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 3);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
};

//----------------------------------------------------------------------------//
// 4 arguments.
template<typename R, typename T1, typename T2, typename T3, typename T4>
struct FunctionTraitsHelper<R(*)(T1, T2, T3, T4)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 4);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
	typedef T4 Arg4Type;
};

//----------------------------------------------------------------------------//
// 5 arguments.
template<typename R, typename T1, typename T2, typename T3, typename T4,
	typename T5>
	struct FunctionTraitsHelper<R(*)(T1, T2, T3, T4, T5)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 5);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
	typedef T4 Arg4Type;
	typedef T5 Arg5Type;
};

//----------------------------------------------------------------------------//
// 6 arguments.
template<typename R, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6>
	struct FunctionTraitsHelper<R(*)(T1, T2, T3, T4, T5, T6)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 6);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
	typedef T4 Arg4Type;
	typedef T5 Arg5Type;
	typedef T6 Arg6Type;
};

//----------------------------------------------------------------------------//
// 7 arguments.
template<typename R, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7>
	struct FunctionTraitsHelper<R(*)(T1, T2, T3, T4, T5, T6, T7)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 7);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
	typedef T4 Arg4Type;
	typedef T5 Arg5Type;
	typedef T6 Arg6Type;
	typedef T7 Arg7Type;
};

//----------------------------------------------------------------------------//
// 8 arguments.
template<typename R, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8>
	struct FunctionTraitsHelper<R(*)(T1, T2, T3, T4, T5, T6, T7, T8)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 8);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
	typedef T4 Arg4Type;
	typedef T5 Arg5Type;
	typedef T6 Arg6Type;
	typedef T7 Arg7Type;
	typedef T8 Arg8Type;
};

//----------------------------------------------------------------------------//
// 9 arguments.
template<typename R, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9>
	struct FunctionTraitsHelper<R(*)(T1, T2, T3, T4, T5, T6, T7, T8, T9)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 9);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
	typedef T4 Arg4Type;
	typedef T5 Arg5Type;
	typedef T6 Arg6Type;
	typedef T7 Arg7Type;
	typedef T8 Arg8Type;
	typedef T9 Arg9Type;
};

//----------------------------------------------------------------------------//
// 10 arguments.
template<typename R, typename T1, typename T2, typename T3, typename T4,
	typename T5, typename T6, typename T7, typename T8, typename T9,
	typename T10>
	struct FunctionTraitsHelper<R(*)(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)>
{
	UT_INCLASS_STATIC_CONSTANT(unsigned, arity = 10);
	typedef R ReturnType;
	typedef T1 Arg1Type;
	typedef T2 Arg2Type;
	typedef T3 Arg3Type;
	typedef T4 Arg4Type;
	typedef T5 Arg5Type;
	typedef T6 Arg6Type;
	typedef T7 Arg7Type;
	typedef T8 Arg8Type;
	typedef T9 Arg9Type;
	typedef T10 Arg10Type;
};

//----------------------------------------------------------------------------//
// Use ut::FunctionTraits to define number of arguments (arity), return type
// and type of the every argument of the provided function signature.
// Example:
// int a = FunctionTraits<int(float,bool)>::arity; // 'a' has value '2'
// FunctionTraits<int(float,bool)>::ReturnType r;  // 'r' has type 'int'
// FunctionTraits<int(float,bool)>::Arg1Type arg1; // 'arg1' has type 'float'
// FunctionTraits<int(float,bool)>::Arg2Type arg2; // 'arg2' has type 'bool'
template<typename Function>
struct FunctionTraits : public FunctionTraitsHelper<typename AddPointer<Function>::Type>
{ };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//