//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_is_same.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
template<typename DesiredType, typename HeadType, typename... TailItems>
struct EachIsImpl
{
	static constexpr bool value = IsSame<DesiredType, HeadType>::value && EachIsImpl<TailItems...>::value;
};

template<typename DesiredType, typename HeadType>
struct EachIsImpl<DesiredType, HeadType>
{
	static constexpr bool value = IsSame<DesiredType, HeadType>::value;
};

// If all types in @Types list name the same type as @DesiredType, provides the
// member constant value equal to true. Otherwise value is false.
template<typename DesiredType, typename... Types>
using EachIs = EachIsImpl<DesiredType, Types...>;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//