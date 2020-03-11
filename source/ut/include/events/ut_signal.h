//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
#include "containers/ut_array.h"
#include "templates/ut_function_traits.h"
#include "templates/ut_container.h"
#include "templates/ut_function.h"
#include "templates/ut_default_combiner.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::SignalTemplate is a template class implementing 'operator ()'
// according to the number of arguments in the provided signature.
template<typename Combiner, typename FunctionSignature> class SignalTemplate;

//----------------------------------------------------------------------------//
// ut::SignalBase is a base (parent) class for all ut::SignalTemplate
// specializations, it implements the main signal inerface (Connect() and Call()
// behaviour), but has no 'operator ()' with variadic number of arguments.
// This operator is implemented in ut::SignalTemplate specializations. And every
// such specialization is inherited from ut::SignalBase class.
template <typename FunctionPtr, typename Combiner>
class SignalBase
{
public:
	// Type of the function signature, but with no pointer.
	typedef typename RemovePointer<FunctionPtr>::Type Signature;

	// Bind a slot using function pointer.
	void Connect(FunctionPtr f)
	{
		slots.Add(Function<Signature>(f));
	}

	// Bind a slot using ut::Function object.
	void Connect(Function<Signature> f)
	{
		slots.Add(Move(f));
	}

protected:
	// Array of slots. Google 'Signals and slots' and 'observer' pattern.
	Array< Function<Signature> > slots;
};

//----------------------------------------------------------------------------//
// Specialized version of the ut::SignalTemplate. Return type and arguments
// are being extracted here from the signature.
template <typename Combiner, typename R, typename... Arguments>
class SignalTemplate<Combiner, R(*)(Arguments...)>
	: public SignalBase<R(*)(Arguments...), Combiner>
{
	typedef SignalBase<R(*)(Arguments...), Combiner> Base;
public:
	Result<R, Error> operator()(Arguments... arguments)
	{
		// Create combiner object
		Combiner combiner;

		// Iterate all slots.
		const size_t slot_count = Base::slots.GetNum();
		for (size_t i = 0; i < slot_count; i++)
		{
			if (i == slot_count - 1)
			{
				// Return only for the last slot.
				return combiner(Base::slots[i](arguments...));
			}
			else
			{
				combiner(Base::slots[i](arguments...));
			}
		}

		// Return the 'Empty' error code in case we
		// have no binded slots.
		return MakeError(error::empty);
	}
};

// Specialized version of the ut::SignalTemplate where return type is void.
template <typename Combiner, typename... Arguments>
class SignalTemplate<Combiner, void(*)(Arguments...)>
	: public SignalBase<void(*)(Arguments...), Combiner>
{
	typedef SignalBase<void(*)(Arguments...), Combiner> Base;
public:
	Result<void, Error> operator()(Arguments... arguments)
	{
		const size_t slot_count = Base::slots.GetNum();
		for (size_t i = 0; i < slot_count; i++)
		{
			Base::slots[i](arguments...); // no need to use combiner
			if (i == slot_count - 1)
			{
				return Result<void, Error>();
			}
		}
		return MakeError(error::empty);
	}
};

//----------------------------------------------------------------------------//
// ut::Signal is a class implementing 'Signals and slots' pattern.
// Use ut::Signal::Connect() to bind a slot, use 'operator ()' for call.
template
<	
	typename FunctionSignature,
	typename Combiner = DefaultCombiner<typename FunctionTraits<FunctionSignature>::ReturnType>
>
class Signal : public SignalTemplate<Combiner, typename AddPointer<FunctionSignature>::Type>
{ };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//