//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// class ut::Ref  is a class template that wraps a reference in a copyable,
// assignable object. It is frequently used as a mechanism to store references
// inside standard containers which cannot normally hold references.
template<typename T>
class Ref
{
public:
	// Constructor, saves pointer to the object @ref
	Ref(T& ref) : ptr(&ref)
	{ }

	// Copy constructor
	Ref(const Ref& ref) : ptr(ref.ptr)
	{ }

	// Conversion operator
	operator T& () const
	{
		return *ptr;
	}

	// Returns a reference to the managed object
	T& Get()
	{
		return *ptr;
	}

	// Overloaded inheritance operator, provides access to the owned object
	T* operator -> ()
	{
		return ptr;
	}

	// Overloaded inheritance operator, provides read access to the owned object
	const T* operator -> () const
	{
		return ptr;
	}

private:
	// pointer to the managed object
	T* ptr;
};

// class ut::ConstRef is the same as ut::Ref but with 'constant' modifier
template<typename T>
class ConstRef
{
public:
	// Constructor, saves pointer to the object @ref
	ConstRef(const T& ref) : ptr(&ref)
	{ }

	// Copy constructor
	ConstRef(const ConstRef& ref) : ptr(ref.ptr)
	{ }

	// Conversion operator
	operator const T& () const
	{
		return *ptr;
	}

	// Returns a reference to the managed object
	const T& Get()
	{
		return *ptr;
	}

	// Overloaded inheritance operator, provides read access to the owned object
	const T* operator -> () const
	{
		return ptr;
	}

private:
	// pointer to the managed object
	const T* ptr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//