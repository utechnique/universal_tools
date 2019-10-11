//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/linkage/ut_meta_shared_holder.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Holds shared pointer to the shared object and address of this object.
struct OutputSharedCacheElement
{
	SharedPtr<SharedPtrHolderBase> ptr;
	const void* address;
};

// Holds shared pointer to the shared object and id of this object.
struct InputSharedCacheElement
{
	SharedPtr<SharedPtrHolderBase> ptr;
	size_t id;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//