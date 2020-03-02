//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_container.h"
#include "ut_function.h"
#include "templates/ut_int_sequence.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::BaseTask is an abstract template class, parent for all tasks with the
// same return type.
template<typename ReturnType>
class BaseTask
{
public:
	virtual ReturnType Execute() = 0;
	virtual ~BaseTask() {}
};

// ut::Task is a template class that is capable to hold desired function and
// it's arguments to call this function in the future.
template<typename Signature> class Task;

//----------------------------------------------------------------------------//
#if CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// Specialization of the ut::Task template class, where
// return type and arguments are separated from the signature.
template<typename R, typename... Arguments>
class Task<R(Arguments...)> : public BaseTask<R>
{
	// Type of the managed function.
	typedef Function<R(Arguments...)> FunctionType;

	// Type of the container for the function arguments.
	typedef Container<Arguments...> ContainerType;

public:
	// Constructor.
	Task()
	{}

	// Constructor.
	//    @param f - function to be called in future.
	//    @param arguments - arguments for the @f function.
	Task(FunctionType f, Arguments... arguments) : function(Move(f))
	                                             , arg_list(Forward<Arguments>(arguments)...)
	{}

	// Calls managed function.
	R Execute()
	{
		return Call(typename MakeIndexSequence<sizeof...(Arguments)>::Type());
	}

private:
	// Helper function to call managed function using pack expansion.
	template<int ...S>
	inline R Call(IntSequence<S...>)
	{
		return function(Forward<typename ContainerType::template Item<S>::Type>(arg_list.template Get<S>())...);
	}

	// managed function
	FunctionType function;

	// container for the function arguments
	ContainerType arg_list;
};

//----------------------------------------------------------------------------//
#else // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// Some macros to declare all specialized versions of the ut::Task template
// using UT_PP_ENUM() macro. Here in comments 'j' is iteration index.
// 1: , Argj
#define UT_TASK_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) Arg##id
// 2: , typename Argj
#define UT_TASK_FULL_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename Arg##id
// 3: , Argj argj
#define UT_TASK_FULL_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) Arg##id arg##id
// 4: , argj
#define UT_TASK_ARG_NAMES_ITERATOR(id) UT_PP_COMMA_IF(id) arg##id
// 5: , arg_list.Get<j>()
#define UT_TASK_ARG_CONTAINER_GET(id) UT_PP_COMMA_IF(id) arg_list.template Get<id>()
// 6: Arg0, Arg1.. , Argj
#define UT_TASK_ARG_LIST(id) UT_PP_ENUM_IN(id, UT_TASK_TYPENAME_ITERATOR)
// 7: typename R, Arg0, Arg1.. , Argj
#define UT_TASK_TEMPLATE_TYPE_LIST(id) \
	typename R UT_PP_COMMA_IF(id) UT_PP_ENUM_IN(id, UT_TASK_FULL_TYPENAME_ITERATOR)
// 8: Arg0 arg0, Arg1 arg1.. , Argj argj
#define UT_TASK_FULL_ARG_LIST(id) UT_PP_ENUM_IN(id, UT_TASK_FULL_ARG_ITERATOR)
// 9: arg0, arg1.. , argj
#define UT_TASK_ARG_NAME_LIST(id) UT_PP_ENUM_IN(id, UT_TASK_ARG_NAMES_ITERATOR)
// 10: arg_list.Get<0>(), arg_list.Get<1>().. , arg_list.Get<j>()
#define UT_TASK_ARG_CONTAINER_LIST(id) UT_PP_ENUM_IN(id, UT_TASK_ARG_CONTAINER_GET)

//----------------------------------------------------------------------------->
// Macro for declaring specialized versions of the ut::Task class.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
// template<typename R, typename T0, typename T1, ... typename Tn>
// class Task<R(T0, T1, ... Tn)> : public BaseTask<R>
// {
//     typedef Function<R(T0, T1, ... Tn)> FunctionType;
//     typedef Container<T0, T1, ... Tn> ContainerType;
// public:
//     Task() {}
//     Task(const FunctionType& f, T0 t0, T1 t1, ... Tn tn) : function(f), arg_list(t0, t1, ... tn) {}
//     R Execute()
//     {
//         return function(arg_list.Get<0>(), arg_list.Get<1>() ... arg_list.Get<n>());
//     }
// private:
//     FunctionType function;
//     ContainerType arg_list;
// };
#define UT_TASK_TEMPLATE_SPECIALIZATION(id)                                    \
template<UT_TASK_TEMPLATE_TYPE_LIST(id)>                                       \
class Task<R(UT_TASK_ARG_LIST(id))> : public BaseTask<R>                       \
{                                                                              \
    typedef Function<R(UT_TASK_ARG_LIST(id))> FunctionType;                    \
    typedef Container<UT_TASK_ARG_LIST(id)> ContainerType;                     \
public:                                                                        \
    Task() {}                                                                  \
    Task(const FunctionType& f UT_PP_COMMA_IF(id) UT_TASK_FULL_ARG_LIST(id)) : \
        function(f), arg_list(UT_TASK_ARG_NAME_LIST(id))                       \
    {}                                                                         \
    R Execute()                                                                \
    {                                                                          \
        return function(UT_TASK_ARG_CONTAINER_LIST(id));                       \
    }                                                                          \
private:                                                                       \
    FunctionType function;                                                     \
    ContainerType arg_list;                                                    \
};

//----------------------------------------------------------------------------->
// Implementation code for all template specializations.
UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_TASK_TEMPLATE_SPECIALIZATION)

//----------------------------------------------------------------------------//
#endif // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//