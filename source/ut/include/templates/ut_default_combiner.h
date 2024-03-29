//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "preprocessor/ut_def.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::DefaultCombiner is a default combiner that returns the last
// returned value from the last call.
template<typename T> struct DefaultCombiner
{
	T operator()(const T& element) const
	{
		return element;
	}
};

// Specialization of the ut::DefaultCombiner template for 'void' type.
template<> struct DefaultCombiner<void>
{
	void operator()() const
	{ } // nothing to return
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//