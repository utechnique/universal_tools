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
// ut::SignalTemplate is a template class implementing 'operator ()'
// according to the number of arguments in the provided signature.
template<typename FunctionSignature, typename Combiner> class SignalTemplate;

//----------------------------------------------------------------------------//
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

//----------------------------------------------------------------------------//
// ut::SignalBase is a base (parent) class for all ut::SignalTemplate
// specializations, it implements the main signal inerface (Connect() and Call()
// behaviour), but has no 'operator ()' with variadic number of arguments.
// This operator is implemented in ut::SignalTemplate specializations. And every
// such specialization is inherited from ut::SignalBase class.
template <typename FunctionPtr, typename Combiner>
class SignalBase
{
	// Type of the inheritor.
	typedef SignalTemplate<FunctionPtr, Combiner> ChildType;
	// Type of the function signature, but with no pointer.
	typedef typename RemovePointer<FunctionPtr>::Type Signature;
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

private:
	// Array of slots. Google 'Signals and slots' and 'observer' pattern.
	Array< Function<Signature> > slots;

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

public:
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
};

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
#define UT_SIGNAL_ARG_NAMES_ITERATOR(id) UT_PP_COMMA_IF(id) arg##id
// 5: , bf->vj
#define UT_SIGNAL_COPIED_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) bf->v##id
// 6: Arg0, Arg1, ... Argj
#define UT_SIGNAL_ARGUMENTS_LIST(id) \
	UT_PP_ENUM_IN(id, UT_SIGNAL_ARG_ITERATOR)
// 7: j, Arg0, Arg1, ... Argj
#define UT_SIGNAL_CONTAINER_TYPE(id) \
	id UT_PP_COMMA_IF(id) UT_SIGNAL_ARGUMENTS_LIST(id)
// 8: R(*)(Arg0, Arg1, ... Argj)
#define UT_SIGNAL_SIGNATURE(id) \
	R(*)(UT_PP_ENUM_IN(id, UT_SIGNAL_ARG_ITERATOR))
// 9: arg0, arg1, ... argj
#define UT_SIGNAL_ARG_NAMES_LIST(id) \
	UT_PP_ENUM_IN(id, UT_SIGNAL_ARG_NAMES_ITERATOR)
// 10: bf->v0, bf->v1, ... bf->vj
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
// template <typename T0, typename T1, ... typename Tn, typename Combiner>
// class SignalTemplate<R(*)(T0, T1, ... Tn), Combiner>
//     : public SignalBase<T0, T1, ... Tn, Combiner>
// {
//     typedef SignalBase<R(*)(T0, T1, ... Tn), Combiner> Base;
//     Container<n, T0, T1, ... Tn>* bf;
// public:
//     Result<R, Error> operator()(T0 t0, T1 t1, ... Tn tn)
//     {
//         Container<n, T0, T1, ... Tn> arguments =
//		       Container<n, T0, T1, ... Tn>(t0, t1, ... tn);
//         bf = &arguments;
//         return Base::Call(&SignalTemplate::Callback);
//     }
// private:
//     R Callback(const Function<R(T0, T1, ... Tn)>& callback) const
//     {
//         return callback(bf->v0, bf->v1, ... bf->vn);
//     }
// };
//
#define UT_SIGNAL_SPECIALIZATION(id)												\
template<UT_SIGNAL_TEMPLATE_TYPE_LIST(id), typename Combiner>						\
class SignalTemplate<UT_SIGNAL_SIGNATURE(id), Combiner>								\
	: public SignalBase<UT_SIGNAL_SIGNATURE(id), Combiner>							\
{																					\
	typedef SignalBase<UT_SIGNAL_SIGNATURE(id), Combiner> Base;						\
	Container<UT_SIGNAL_CONTAINER_TYPE(id)>* bf;									\
public:																				\
	Result<R, Error> operator()(UT_SIGNAL_FULL_ARG_LIST(id))						\
	{																				\
		Container<UT_SIGNAL_CONTAINER_TYPE(id)>	arguments = 						\
			Container<UT_SIGNAL_CONTAINER_TYPE(id)>(UT_SIGNAL_ARG_NAMES_LIST(id));	\
		bf = &arguments;															\
		return Base::Call(&SignalTemplate::Callback);								\
	}																				\
private:																			\
	R Callback(const Function<R(UT_SIGNAL_ARGUMENTS_LIST(id))>& callback) const		\
	{																				\
		return callback(UT_SIGNAL_COPIED_ARGUMENTS_LIST(id));						\
	}																				\
};

//----------------------------------------------------------------------------//
// All specialized variations are declared here.
UT_PP_ENUM(UT_FUNCTION_TRAITS_MAX_ARITY, UT_SIGNAL_SPECIALIZATION)

//----------------------------------------------------------------------------//
// ut::Signal is a class implementing 'Signals and slots' pattern.
// Use ut::Signal::Connect() to bind a slot, use 'operator ()' for call.
template
<	
	typename FunctionSignature,
	typename Combiner = DefaultSignalCombiner<typename FunctionTraits<FunctionSignature>::ReturnType>
>
class Signal : public SignalTemplate<typename AddPointer<FunctionSignature>::Type, Combiner>
{ };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//