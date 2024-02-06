//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "preprocessor/ut_def.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// The class template ut::IntSequence represents a compile-time sequence of
// integers. When used as an argument to a function template, the parameter pack
// Ints can be deduced and used in pack expansion.
template<int...>
struct IntSequence {};

template<int N, int... S>
struct MakeIndexSequence : MakeIndexSequence<N - 1, N - 1, S...> {};

template<int... S>
struct MakeIndexSequence<0, S...>{ typedef IntSequence<S...> Type; };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//