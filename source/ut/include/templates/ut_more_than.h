//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
#include "ut_integral_constant.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
template<int cmp, int iterator, typename HeadType, typename... TailItems>
struct MoreThanImpl
{
	static constexpr bool value = MoreThanImpl<cmp, iterator + 1, TailItems...>::value;
};

template<int cmp, int iterator, typename HeadType>
struct MoreThanImpl<cmp, iterator, HeadType>
{
	static constexpr bool value = iterator >= cmp;
};

template<int cmp, typename... Types>
using MoreThan = MoreThanImpl<cmp, 0, Types...>;
//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//