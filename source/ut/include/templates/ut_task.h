//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_tuple.h"
#include "ut_function.h"
#include "ut_int_sequence.h"
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
	virtual ~BaseTask() = default;
};

// ut::Task is a template class that is capable to hold desired function and
// it's arguments to call this function in the future.
template<typename Signature> class Task;

//----------------------------------------------------------------------------//
// Specialization of the ut::Task template class, where
// return type and arguments are separated from the signature.
template<typename R, typename... Arguments>
class Task<R(Arguments...)> : public BaseTask<R>
{
	// Type of the managed function.
	typedef Function<R(Arguments...)> FunctionType;

	// Type of the tuple for the function arguments.
	typedef Tuple<Arguments...> TupleType;

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
		return function(Forward<typename TupleType::template Item<S>::Type>(arg_list.template Get<S>())...);
	}

	// managed function
	FunctionType function;

	// container for the function arguments
	TupleType arg_list;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//