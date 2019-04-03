//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "preprocessor/ut_enum.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Maximum possible number of elements in container template
// minus one argument for void type.
#define UT_CONTAINER_MAX 10

//----------------------------------------------------------------------------//
// Some macros to declare all specialized versions of the ut::Container
// template using UT_PP_ENUM() macro. Here in comments 'j' is an Id.
// 1: , Tj
#define UT_CONTAINER_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) T##id
// 2: , typename Tj
#define UT_CONTAINER_FULL_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename T##id 
// 3: , typename Tj = void
#define UT_CONTAINER_FULL_VOID_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename T##id = void
// 4: , Tj ij
#define UT_CONTAINER_INPUT_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) T##id i##id
// 5: , vn(ij)
#define UT_CONTAINER_INPUT_ARG_INIT_ITERATOR(id) UT_PP_COMMA_IF(id) v##id(i##id)
// 6: Tj vj;
#define UT_CONTAINER_DECLARE_VAR(id) T##id v##id;

//----------------------------------------------------------------------------//
// ut::Container is a wrapper template struct to contain variable number of
// arguments. Here is declared base template. Lower in this file it will be
// specialized according to the number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <int arity = 0, typename T0 = void, typename T1 = void ...>
// struct Container
// {
// };
template < int arity = 0,
	UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_FULL_VOID_TYPENAME_ITERATOR)
> struct Container {};

//----------------------------------------------------------------------------//
// Macro for declaring specialized versions of the ut::Container template.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename T0, typename T1, ... typename T(j+1) >
// struct Container<j + 1, T0, T1, ... T(j + 1)>
// {
//     Container(T0 i0, T1 i1, ... Tj ij) : v0(i0), v1(i1), ... vj(ij) {}
//     T0 v0;
//     T1 v1;
//     ...
//     Tj vj;
// };
#define UT_CONTAINER_SPECIALIZATION(id)							\
template <														\
	UT_PP_ENUM_IN(id, UT_CONTAINER_FULL_TYPENAME_ITERATOR)		\
>																\
struct Container<												\
	id UT_PP_COMMA_IF(id)										\
	UT_PP_ENUM_IN(id, UT_CONTAINER_TYPENAME_ITERATOR)			\
>																\
{																\
	Container(													\
		UT_PP_ENUM_IN(id, UT_CONTAINER_INPUT_ARG_ITERATOR)		\
	) UT_PP_COLON_IF(id)										\
		UT_PP_ENUM_IN(id, UT_CONTAINER_INPUT_ARG_INIT_ITERATOR)	\
	{}															\
	UT_PP_ENUM_IN(id, UT_CONTAINER_DECLARE_VAR)					\
};

//----------------------------------------------------------------------------//
// All specialized variations are declared here.
UT_PP_ENUM(UT_CONTAINER_MAX, UT_CONTAINER_SPECIALIZATION)

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//