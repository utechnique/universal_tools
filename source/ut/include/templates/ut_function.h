//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
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

// ut::FunctorInvoker contains intermediate object that has '()' operator.
template <typename Functor, typename Signature> class FunctorInvoker;

// ut::FunctionTemplate is a template class implementing 'operator ()'
// according to the number of arguments in the provided signature.
template <typename Signature> class FunctionTemplate;

//----------------------------------------------------------------------------//
// ut::FunctionBase is a base (parent) class for all ut::FunctionTemplate
// specializations, it contains unique pointer to the invoker object, performs
// copying and initialization of the invoker object (look for ut::Invoker).
template<typename Signature>
class FunctionBase
{
public:
	// Constructor, creates a copy of the provided invoker object.
	FunctionBase(UniquePtr< Invoker<Signature> > in_invoker) : invoker(Move(in_invoker))
	{ }

	// Copy constructor, copies invoker object from the source.
	FunctionBase(const FunctionBase& copy) : invoker(copy.invoker->MakeCopy())
	{ }

	// Assignment operator, copies invoker object from the source.
	FunctionBase& operator = (const FunctionBase& copy)
	{
		invoker = UniquePtr< Invoker<Signature> >(copy.invoker->MakeCopy());
		return *this;
	}

protected:
	// Invoker performs correct call of the managed function.
	UniquePtr< Invoker<Signature> > invoker;
};

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
		return function(Forward<Arguments>(arguments)...);
	}

	virtual Invoker* MakeCopy() const
	{
		return new Invoker(*this);
	}

	virtual bool IsValid() const
	{
		return function != nullptr;
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
		return (owner->*member)(Forward<Arguments>(arguments)...);
	}

	Base* MakeCopy() const
	{
		return new MemberInvoker(*this);
	}

	bool IsValid() const
	{
		return owner != nullptr;
	}

private:
	MemberFunctionPtr member;
	C* owner;
};

// Specialization of the ut::FunctorInvoker template class, where
// return type and arguments are separated from the signature.
template <typename Functor, typename R, typename... Arguments>
class FunctorInvoker<Functor, R(*)(Arguments...)> : public Invoker<R(*)(Arguments...)>
{
	typedef Invoker<R(*)(Arguments...)> Base;
public:
	FunctorInvoker(Functor in_functor) : functor(in_functor) {}

	virtual R Invoke(Arguments... arguments) const
	{
		return functor(Forward<Arguments>(arguments)...);
	}

	virtual Base* MakeCopy() const
	{
		return new FunctorInvoker(*this);
	}

	virtual bool IsValid() const
	{
		return true;
	}

private:
	Functor functor;
};

//----------------------------------------------------------------------------//
// Specialization of the ut::FunctionTemplate template class, where
// return type and arguments are separated from the signature.
template <typename R, typename... Arguments>
class FunctionTemplate<R(*)(Arguments...)> : public FunctionBase<R(*)(Arguments...)>
{
private:
	typedef R(*FunctionPtr)(Arguments...);
	typedef FunctionBase<R(*)(Arguments...)> Base;
	FunctionPtr function;

public:
	template<typename ClassType> using MemberFunction = R(ClassType::*)(Arguments...);

	FunctionTemplate(UniquePtr< Invoker<FunctionPtr> > invoker) : Base(Move(invoker))
	{}

	R operator ()(Arguments... arguments) const
	{
		return Base::invoker->Invoke(Forward<Arguments>(arguments)...);
	}
};

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
	Function(UniquePtr< Invoker<Pointer> > invoker) : Base(Move(invoker))
	{}

	Function(Pointer ptr = nullptr) : Base(MakeUnique< Invoker<Pointer> >(ptr))
	{}

	template<typename Functor>
	Function(Functor functor) : Base(MakeUnique< FunctorInvoker<Functor, Pointer> >(Move(functor)))
	{}

	bool IsValid() const
	{
		return this->invoker->IsValid();
	}
};

// Convenient function to create ut::Function from a pointer to member function.
// 'C' - class type, 'S' - function signature.
template<typename C, typename S>
inline Function<S> MemberFunction(C* obj, typename Function<S>::template MemberFunction<C> ptr)
{
	UniquePtr< Invoker<typename AddPointer<S>::Type> > invoker
		(MakeUnique< MemberInvoker<typename Function<S>::template MemberFunction<C> > >(ptr, obj));
	return Function<S>(Move(invoker));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//