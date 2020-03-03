//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_def.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// The class template ut::IntSequence represents a compile-time sequence of
// integers. When used as an argument to a function template, the parameter pack
// Ints can be deduced and used in pack expansion.
#if CPP_STANDARD >= 2011
template<int...>
struct IntSequence {};

template<int N, int... S>
struct MakeIndexSequence : MakeIndexSequence<N - 1, N - 1, S...> {};

template<int... S>
struct MakeIndexSequence<0, S...>{ typedef IntSequence<S...> Type; };
#endif
//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//