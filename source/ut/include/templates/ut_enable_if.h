//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::EnableIf template metafunction is a convenient way to leverage SFINAE to 
// conditionally remove functions from overload resolution based on type traits
// and to provide separate function overloads and specializations for different
// type ut::EnableIf<> can be used as an additional function argument (not
// applicable to operator overloads), as a return type(not applicable to
// constructors), or as a class template or function template parameter.
template<bool B, class T = void>
struct EnableIf {};

template<class T>
struct EnableIf<true, T> { typedef T Type; };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//