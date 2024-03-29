//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "preprocessor/ut_def.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// If A and B name the same type, provides the member constant value equal to
// true. Otherwise value is false.
template<typename A, typename B>
struct IsSame { static constexpr bool value = false; };

template<typename A>
struct IsSame<A, A> { static constexpr bool value = true; };

// If A and B name the same template, provides the member constant value equal to
// true. Otherwise value is false.
template <class A, class B>
struct IsSameTemplate : IsSame<A, B>
{};

template <template<class...> class T, class A, class B>
struct IsSameTemplate<T<A>, T<B>> { static constexpr bool value = true; };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//