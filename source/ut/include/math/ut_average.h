//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "templates/ut_enable_if.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Computes the average of a set of integer values.
template<typename ValueType>
typename EnableIf<IsIntegral<ValueType>::value, ValueType>::Type Average(const ValueType* elements, size_t count)
{
	ValueType x = 0;
	ValueType y = 0;
	ValueType n = static_cast<ValueType>(count);
	for (size_t i = 0; i < count; i++)
	{
		x += elements[i] / n;
		ValueType b = elements[i] % n;
		if (y >= n - b)
		{
			x++;
			y -= n - b;
		}
		else
		{
			y += b;
		}
	}

	return x + y / n;
}

// Computes the average of a set of floating point values.
template<typename ValueType>
typename EnableIf<!IsIntegral<ValueType>::value, ValueType>::Type Average(const ValueType* elements, size_t count)
{
	const ValueType d = static_cast<ValueType>(count);
	ValueType out = static_cast<ValueType>(0);
	for (size_t i = 0; i < count; i++)
	{
		out += elements[i] / d;
	}
	return out;
}

// Computes the average of a static array.
template<typename ValueType, size_t count> ValueType Average(const ValueType(&elements)[count])
{
	return Average<ValueType>(&elements[0], count);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//