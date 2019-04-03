//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Pair is a struct template that provides a way to store two
// heterogeneous objects as a single unit.
template <typename T1, typename T2>
class Pair
{
public:
	// Default constructor
	Pair()
	{ }

	// Constructor, both arguments are const references
	Pair(const T1& t1, const T2& t2) : first(t1), second(t2)
	{ }

#if CPP_STANDARD >= 2011
	// Constructor, both arguments are r-value references
	Pair(T1 && t1, T2 && t2) : first(Move(t1)), second(Move(t2))
	{ }

	// Constructor, first argument is l-value reference,
	// second one is r-value reference
	Pair(const T1& t1, T2 && t2) : first(t1), second(Move(t2))
	{ }

	// Constructor, first argument is r-value reference,
	// second one is l-value reference
	Pair(T1 && t1, const T2& t2) : first(Move(t1)), second(t2)
	{ }
#endif

	// Copy constructor
	Pair(const Pair& copy) : first(copy.first), second(copy.second)
	{ }

	// Move constructor
#if CPP_STANDARD >= 2011
	Pair(Pair && copy) : first(Move(copy.first)), second(Move(copy.second))
	{ }
#endif

	// Assignment operator
	Pair& operator = (const Pair& copy)
	{
		first = copy.first;
		second = copy.second;
		return *this;
	}

	// Move operator
#if CPP_STANDARD >= 2011
	Pair& operator = (Pair && copy)
	{
		first = Move(copy.first);
		second = Move(copy.second);
		return *this;
	}
#endif

	// managed objects
	T1 first;
	T2 second;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//