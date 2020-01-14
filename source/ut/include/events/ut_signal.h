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
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::DefaultSignalCombiner is a default signal combiner that returns the last
// returned value from the last slot call.
template <typename T> struct DefaultSignalCombiner
{
	T operator()(const T& element) const
	{
		return element;
	}
};

// Specialization of the ut::DefaultSignalCombiner template for 'void' type.
template <> struct DefaultSignalCombiner<void>
{
	void operator()(void) const
	{ } // nothing to return
};

//----------------------------------------------------------------------------//
#if CPP_STANDARD < 2011
// ut::SlotInvoker class makes possible to call slots that return void type.
// This is done by specializing SlotInvoker template for 'void' type.
template <typename ReturnType,
          typename ChildType,
          typename CallBackType,
          typename Signature,
          typename Combiner>
class SlotInvoker
{
	template <typename, typename> friend class SignalBase;
	static FORCEINLINE Result<ReturnType, Error> Call(const ChildType* child,
	                                                  const Function<Signature>& slot,
	                                                  const CallBackType callback,
	                                                  Combiner& combiner)
	{
		return combiner((child->*callback)(slot));
	}
};

// ut::SlotInvoker template specialization for void type.
template <typename ChildType, typename CallBackType, typename Signature, typename Combiner>
class SlotInvoker<void, ChildType, CallBackType, Signature, Combiner>
{
	template <typename, typename> friend class SignalBase;
	static FORCEINLINE Result<void, Error> Call(const ChildType* child,
	                                            const Function<Signature>& slot,
	                                            const CallBackType callback,
	                                            Combiner& combiner)
	{
		// call slot at first
		(child->*callback)(slot);

		// and return empty result
		return Result<void, Error>();
	}
};
#endif // CPP_STANDARD < 2011

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
	void Connect(const Function<Signature>& f)
	{
		slots.Add(f);
	}

protected:
	// Array of slots. Google 'Signals and slots' and 'observer' pattern.
	Array< Function<Signature> > slots;

private:
#if CPP_STANDARD < 2011
	// Type of the inheritor.
	typedef SignalTemplate<Combiner, FunctionPtr> ChildType;
	// Return type of the signature.
	typedef typename FunctionTraits<Signature>::ReturnType ReturnType;
	// Type of the inheritor's callback. This callback is used to be able to call
	// functions with variadic number of arguments from the one(!) current(!) class
	// implementation.
	typedef ReturnType(ChildType::*CallBackType)(const Function<Signature>&) const;
	// Invoker allows to use callbacks that return void type.
	typedef SlotInvoker<ReturnType, ChildType, CallBackType, Signature, Combiner> Invoker;
	// Make inheritor the only external class that is able to call SignalBase::Call().
	friend ChildType;

	// Calls every slot and return the result.
	//    @param callback - inheritor's callback.
	//    @return - return value of the last slot,
	//              or error if slot array is empty.
	Result<ReturnType, Error> Call(const CallBackType callback) const
	{
		// It's ok to cast directly to the inheritor, because this
		// method is private and only @ChildType is a friend.
		const ChildType* child = static_cast<const ChildType*>(this);

		// Create combiner object
		Combiner combiner;

		// Iterate all slots.
		for (size_t i = 0; i < slots.GetNum(); i++)
		{
			if (i == slots.GetNum() - 1)
			{
				// Return only for the last slot.
				return Invoker::Call(child, slots[i], callback, combiner);
			}
			else
			{
				Invoker::Call(child, slots[i], callback, combiner);
			}
		}

		// Return the 'Empty' error code in case we
		// have no binded slots.
		return MakeError(error::empty);
	}
#endif // CPP_STANDARD < 2011
};

//----------------------------------------------------------------------------//
#if CPP_STANDARD >= 2011
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
#else // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// Some macros to declare all specialized versions of ut::SignalTemplate
// class using UT_PP_ENUM() macro. Here in comments 'j' is an Id of a
// template specialization.
// 1: , typename Tj
#define UT_SIGNAL_TYPENAME_DECL_ITERATOR(id) UT_PP_COMMA_IF(id) typename Arg##id
// 2: , Argj
#define UT_SIGNAL_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) Arg##id
// 3: , Argj argj
#define UT_SIGNAL_FULL_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) Arg##id arg##id
// 4: , argj
#define UT_SIGNAL_ARG_NAMES_ITERATOR(id) UT_PP_COMMA_IF(id) Forward<Arg##id>(arg##id)
// 5: , bf->Get<j>()
#define UT_SIGNAL_COPIED_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) bf->template Get<id>()
// 6: Arg0, Arg1, ... Argj
#define UT_SIGNAL_ARGUMENTS_LIST(id) \
	UT_PP_ENUM_IN(id, UT_SIGNAL_ARG_ITERATOR)
// 7: j, Arg0, Arg1, ... Argj
#define UT_SIGNAL_CONTAINER_TYPE(id) \
	UT_SIGNAL_ARGUMENTS_LIST(id)
// 8: R(*)(Arg0, Arg1, ... Argj)
#define UT_SIGNAL_SIGNATURE(id) \
	R(*)(UT_PP_ENUM_IN(id, UT_SIGNAL_ARG_ITERATOR))
// 9: arg0, arg1, ... argj
#define UT_SIGNAL_ARG_NAMES_LIST(id) \
	UT_PP_ENUM_IN(id, UT_SIGNAL_ARG_NAMES_ITERATOR)
// 10: bf->Get<0>(), bf->Get<1>(), ... bf->Get<j>()
#define UT_SIGNAL_COPIED_ARGUMENTS_LIST(id) \
	UT_PP_ENUM_IN(id, UT_SIGNAL_COPIED_ARG_ITERATOR)
// 11: typename R, typename Arg0, typename Arg1, ... typename Argj
#define UT_SIGNAL_TEMPLATE_TYPE_LIST(id) \
	typename R UT_PP_COMMA_IF(id) UT_PP_ENUM_IN(id, UT_SIGNAL_TYPENAME_DECL_ITERATOR)
// 12: Arg0 arg0, Arg1 arg1, ... Argj argj
#define UT_SIGNAL_FULL_ARG_LIST(id) \
	UT_PP_ENUM_IN(id, UT_SIGNAL_FULL_ARG_ITERATOR)

//----------------------------------------------------------------------------//
// Macro for declaring specialized versions of the ut::SignalTemplate class.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename Combiner, typename R, typename T0, typename T1, ... typename Tn>
// class SignalTemplate<Combiner, R(*)(T0, T1, ... Tn)>
//     : public SignalBase<R(*)(T0, T1, ... Tn), Combiner>
// {
//     typedef SignalBase<R(*)(T0, T1, ... Tn), Combiner> Base;
//     Container<T0, T1, ... Tn>* bf;
// public:
//     Result<R, Error> operator()(T0 t0, T1 t1, ... Tn tn)
//     {
//         Container<T0, T1, ... Tn> arguments =
//		       Container<T0, T1, ... Tn>(t0, t1, ... tn);
//         bf = &arguments;
//         return Base::Call(&SignalTemplate::Callback);
//     }
// private:
//     R Callback(const Function<R(T0, T1, ... Tn)>& callback) const
//     {
//         return callback(bf->Get<0>(), bf->Get<1>(), ... bf->Get<n>());
//     }
// };
//
#define UT_SIGNAL_SPECIALIZATION(id)                                                \
template<typename Combiner, UT_SIGNAL_TEMPLATE_TYPE_LIST(id)>                       \
class SignalTemplate<Combiner, UT_SIGNAL_SIGNATURE(id)>                             \
	: public SignalBase<UT_SIGNAL_SIGNATURE(id), Combiner>                          \
{                                                                                   \
	typedef SignalBase<UT_SIGNAL_SIGNATURE(id), Combiner> Base;                     \
	Container<UT_SIGNAL_CONTAINER_TYPE(id)>* bf;                                    \
public:                                                                             \
	Result<R, Error> operator()(UT_SIGNAL_FULL_ARG_LIST(id))                        \
	{                                                                               \
		Container<UT_SIGNAL_CONTAINER_TYPE(id)>	arguments =                         \
			Container<UT_SIGNAL_CONTAINER_TYPE(id)>(UT_SIGNAL_ARG_NAMES_LIST(id));  \
		bf = &arguments;                                                            \
		return Base::Call(&SignalTemplate::Callback);                               \
	}                                                                               \
private:                                                                            \
	R Callback(const Function<R(UT_SIGNAL_ARGUMENTS_LIST(id))>& callback) const     \
	{                                                                               \
		return callback(UT_SIGNAL_COPIED_ARGUMENTS_LIST(id));                       \
	}                                                                               \
};

//----------------------------------------------------------------------------//
// All specialized variations are declared here.
UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_SIGNAL_SPECIALIZATION)

//----------------------------------------------------------------------------//
#endif // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// ut::Signal is a class implementing 'Signals and slots' pattern.
// Use ut::Signal::Connect() to bind a slot, use 'operator ()' for call.
template
<	
	typename FunctionSignature,
	typename Combiner = DefaultSignalCombiner<typename FunctionTraits<FunctionSignature>::ReturnType>
>
class Signal : public SignalTemplate<Combiner, typename AddPointer<FunctionSignature>::Type>
{ };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//