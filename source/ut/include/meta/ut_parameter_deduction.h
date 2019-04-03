//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_parameter.h"
#include "meta/ut_string_parameter.h"
#include "meta/ut_ptr_parameter.h"
#include "meta/ut_array_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::DeduceParameter template function serves to automaticly deduce parameter
// type according to the provided argument and return a new instance of this
// parameter.
//    @param ref - reference to the managed object.
//    @return - ut::Ptr object with newly created parameter.
template<typename T>
inline UniquePtr<BaseParameter> DeduceParameter(T& ref)
{
	UniquePtr<BaseParameter> parameter(new Parameter<T>(&ref));
	return Move(parameter);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//