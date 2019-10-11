//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_base_parameter.h"
#include "pointers/ut_shared_ptr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Link is a structure containing a pointer to the parameter which
// can be subsequently linked with another parameter during saving/loading
// a meta snapshot. Also it has unique id that helps to identify this parameter
// during deserialization.
struct Link
{
	SharedPtr<BaseParameter> parameter;
	size_t id;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//