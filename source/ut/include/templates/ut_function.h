//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_container.h"
#include "ut_function_traits.h"
#include "pointers/ut_unique_ptr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Invoker is a class that can call functions with variable arity.
// See ut::Invoker::Invoke function. This template is specialized for every
// possible arity of the function.
template <typename Signature> class Invoker;

// ut::MemberInvoker is the same as ut::Invoker, but for member functions.
template <typename Signature> class MemberInvoker;

// ut::FunctionTemplate is a template class implementing 'operator ()'
// according to the number of arguments in the provided signature.
template <typename Signature> class FunctionTemplate;

//----------------------------------------------------------------------------//
// ut::FunctionBase is a base (parent) class for all ut::FunctionTemplate
// specializations, it contains unique pointer to the invoker object, performs
// copying and initialization of the invoker object (look for ut::Invoker).
template<typename T>
class FunctionBase
{
public:
	// Constructor, creates a copy of the provided invoker object.
	FunctionBase(const Invoker<T>& in_invoker) : invoker(in_invoker.MakeCopy())
	{ }

	// Copy constructor, copies invoker object from the source.
	FunctionBase(const FunctionBase& copy) : invoker(copy.invoker->MakeCopy())
	{ }

protected:
	// Invoker performs correct call of the managed function.
	UniquePtr< Invoker<T> > invoker;
};

//----------------------------------------------------------------------------//
#if CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// Specialization of the ut::Invoker template class, where
// return type and arguments are separated from the signature.
template <typename R, typename... Arguments>
class Invoker<R(*)(Arguments...)>
{
protected:
	typedef R(*FunctionPtr)(Arguments...);
	FunctionPtr function;

public:
	Invoker(FunctionPtr ptr = nullptr) : function(ptr) {}

	virtual R Invoke(Arguments... arguments) const
	{
		return function(arguments...);
	}

	virtual Invoker* MakeCopy() const
	{
		return new Invoker(*this);
	}
};

// Specialization of the ut::MemberInvoker template class, where
// return type, owner class type and arguments are separated from the signature.
template <typename C, typename R, typename... Arguments>
class MemberInvoker<R(C::*)(Arguments...)> : public Invoker<R(*)(Arguments...)>
{
	typedef Invoker<R(*)(Arguments...)> Base;
	typedef R(C::*MemberFunctionPtr)(Arguments...);
public:
	MemberInvoker(MemberFunctionPtr ptr, C* object) : member(ptr)
	                                                , owner(object)
	{}

	R Invoke(Arguments... arguments) const
	{
		return (owner->*member)(arguments...);
	}

	Base* MakeCopy() const
	{
		return new MemberInvoker(*this);
	}

private:
	MemberFunctionPtr member;
	C* owner;
};

// Specialization of the ut::FunctionTemplate template class, where
// return type and arguments are separated from the signature.
template <typename R, typename... Arguments>
class FunctionTemplate<R(*)(Arguments...)> : public FunctionBase<R(*)(Arguments...)>
{
	typedef R(*FunctionPtr)(Arguments...);
	typedef FunctionBase<R(*)(Arguments...)> Base;
	FunctionPtr function;
public:
	FunctionTemplate(const Invoker<FunctionPtr>& invoker) : Base(invoker)
	{}

	R operator ()(Arguments... arguments) const
	{
		return Base::invoker->Invoke(arguments...);
	}
};

//----------------------------------------------------------------------------//
#else // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// Some macros to declare all specialized versions of function container
// templates using UT_PP_ENUM() macro. Here in comments 'j' is an Id of a
// template specialization.
// 1: , Argj
#define UT_FUNCTION_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) Arg##id
// 2: , typename Argj
#define UT_FUNCTION_FULL_TYPENAME_ITERATOR(id) UT_PP_COMMA_IF(id) typename Arg##id
// 3: , Argj argj
#define UT_FUNCTION_FULL_ARG_ITERATOR(id) UT_PP_COMMA_IF(id) Arg##id arg##id
// 4: , argj
#define UT_FUNCTION_ARG_NAMES_ITERATOR(id) UT_PP_COMMA_IF(id) arg##id
// 5: Arg0, Arg1.. , Argj
#define UT_FUNCTION_ARG_LIST(id) UT_PP_ENUM_IN(id, UT_FUNCTION_TYPENAME_ITERATOR)
// 6: typename R, Arg0, Arg1.. , Argj
#define UT_FUNCTION_TEMPLATE_TYPE_LIST(id) \
	typename R UT_PP_COMMA_IF(id) UT_PP_ENUM_IN(id, UT_FUNCTION_FULL_TYPENAME_ITERATOR)
// 7: Arg0 arg0, Arg1 arg1.. , Argj argj
#define UT_FUNCTION_FULL_ARG_LIST(id) UT_PP_ENUM_IN(id, UT_FUNCTION_FULL_ARG_ITERATOR)
// 8: arg0, arg1.. , argj
#define UT_FUNCTION_ARG_NAME_LIST(id) UT_PP_ENUM_IN(id, UT_FUNCTION_ARG_NAMES_ITERATOR)

//----------------------------------------------------------------------------//
// Macro for declaring specialized versions of the ut::Invoker template.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename R, typename T0, typename T1, ... typename Tn>
// class Invoker<R(*)(T0, T1, ... Tn)>
// {
// protected:
//     typedef R(*FunctionPtr)(T0, T1, ... Tn);
//     FunctionPtr function;
// public:
//     Invoker(FunctionPtr ptr = nullptr) : function(ptr) {}
//     virtual R Invoke(T0 t0, T1 t1, ... Tn tn) const
//     {
//         return function(t0, t1, ... tn);
//     }
//     virtual Invoker* MakeCopy() const
//     {
//         return new Invoker(*this);
//     }
// };
//
#define UT_FUNCTION_INVOKER_SPECIALIZATION(id)					\
template<UT_FUNCTION_TEMPLATE_TYPE_LIST(id)>					\
class Invoker<R(*)(UT_FUNCTION_ARG_LIST(id))>					\
{																\
protected:														\
	typedef R(*FunctionPtr)(UT_FUNCTION_ARG_LIST(id));			\
	FunctionPtr function;										\
public:															\
	Invoker(FunctionPtr ptr = nullptr) : function(ptr) {}		\
	virtual R Invoke(UT_FUNCTION_FULL_ARG_LIST(id)) const		\
	{															\
		return function(UT_FUNCTION_ARG_NAME_LIST(id));			\
	}															\
	virtual Invoker* MakeCopy() const							\
	{															\
		return new Invoker(*this);								\
	}															\
};

//----------------------------------------------------------------------------//
// Macro for declaring specialized versions of the ut::MemberInvoker template.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename C, typename R, typename T0, typename T1, ... typename Tn>
// class MemberInvoker<R(C::*)(T0, T1, ... Tn)> : public Invoker<(T0, T1, ... Tn)>
// {
//     typedef Invoker<R(*)(T0, T1, ... Tn)> Base;
//     typedef R(C::*MemberFunctionPtr)(T0, T1, ... Tn);
// public:
//     MemberInvoker(MemberFunctionPtr ptr, C* object) : member(ptr)
//                                                     , owner(object) {}
//     R Invoke(T0 t0, T1 t1, ... Tn tn) const
//     {
//         return (owner->*member)(t0, t1, ... tn);
//     }
//     Base* MakeCopy() const { return new MemberInvoker(*this); }
// private:
//     MemberFunctionPtr member;
//     C* owner;
// };
//
#define UT_MEMBER_FUNCTION_INVOKER_SPECIALIZATION(id)					\
template<typename C, UT_FUNCTION_TEMPLATE_TYPE_LIST(id)>				\
class MemberInvoker<R(C::*)(UT_FUNCTION_ARG_LIST(id))>					\
	: public Invoker<R(*)(UT_FUNCTION_ARG_LIST(id))>					\
{																		\
	typedef Invoker<R(*)(UT_FUNCTION_ARG_LIST(id))> Base;				\
	typedef R(C::*MemberFunctionPtr)(UT_FUNCTION_ARG_LIST(id));			\
public:																	\
	MemberInvoker(MemberFunctionPtr ptr, C* object) : member(ptr)		\
	                                                , owner(object) {}	\
	R Invoke(UT_FUNCTION_FULL_ARG_LIST(id)) const						\
	{																	\
		return (owner->*member)(UT_FUNCTION_ARG_NAME_LIST(id));			\
	}																	\
	Base* MakeCopy() const { return new MemberInvoker(*this); }			\
private:																\
	MemberFunctionPtr member;											\
	C* owner;															\
};

//----------------------------------------------------------------------------//
// Macro for declaring specialized versions of the ut::FunctionTemplate class.
// Every specialized version contains different number of arguments.
// Pseudocode sample without preprocessor macros:
//
// template <typename R, typename T0, typename T1, ... typename Tn>
// class FunctionTemplate<R(*)(T0, T1, ... Tn)>
//     : public FunctionBase<R(*)(T0, T1, ... Tn)>
// {
//     typedef R(*FunctionPtr)(T0, T1, ... Tn);
//     typedef FunctionBase<R(*)(T0, T1, ... Tn)> Base;
//     FunctionPtr function;
// public:
//     FunctionTemplate(const Invoker<FunctionPtr>& invoker) : Base(invoker){}
//     R operator ()(T0 t0, T1 t1, ... Tn tn) const
//     {
//         return Base::invoker->Invoke(t0, t1, ... tn);
//     }
// };
//
#define UT_MEMBER_FUNCTION_TEMPLATE_SPECIALIZATION(id)						\
template<UT_FUNCTION_TEMPLATE_TYPE_LIST(id)>								\
class FunctionTemplate<R(*)(UT_FUNCTION_ARG_LIST(id))>						\
	: public FunctionBase<R(*)(UT_FUNCTION_ARG_LIST(id))>					\
{																			\
	typedef R(*FunctionPtr)(UT_FUNCTION_ARG_LIST(id));						\
	typedef FunctionBase<R(*)(UT_FUNCTION_ARG_LIST(id))> Base;				\
	FunctionPtr function;													\
public:																		\
	FunctionTemplate(const Invoker<FunctionPtr>& invoker) : Base(invoker){}	\
	R operator ()(UT_FUNCTION_FULL_ARG_LIST(id)) const						\
	{																		\
		return Base::invoker->Invoke(UT_FUNCTION_ARG_NAME_LIST(id));		\
	}																		\
};

//----------------------------------------------------------------------------//
// Implementation code for all template specializations.
UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_FUNCTION_INVOKER_SPECIALIZATION)
UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_MEMBER_FUNCTION_INVOKER_SPECIALIZATION)
UT_PP_ENUM(UT_FUNCTION_MAX_ARITY, UT_MEMBER_FUNCTION_TEMPLATE_SPECIALIZATION)

//----------------------------------------------------------------------------//
#endif // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
// ut::Function is a template class to encapsulate different variations of the
// function implementation (such as member-function and/or simple function) in
// one object. It can be considered to be something like smart function pointer.
template <typename FunctionSignature>
class Function : public FunctionTemplate<typename AddPointer<FunctionSignature>::Type>
{
	typedef typename AddPointer<FunctionSignature>::Type Pointer;
	typedef FunctionTemplate<Pointer> Base;
public:
	Function(const Invoker<Pointer>& invoker) : Base(invoker)
	{}

	Function(Pointer ptr = nullptr) : Base(Invoker<Pointer>(ptr))
	{}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//