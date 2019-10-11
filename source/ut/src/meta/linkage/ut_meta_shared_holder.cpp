//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/linkage/ut_meta_shared_holder.h"
#include "meta/ut_meta_controller.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Constructor
//    @param in_address - address of the managed ut::SharedPtr object.
SharedPtrHolderBase::SharedPtrHolderBase(void* in_address) : address(in_address)
{ }

// Returns address of the managed ut::SharedPtr object.
void* SharedPtrHolderBase::GetAddress()
{
	return address;
}

// Virtual destructor.
SharedPtrHolderBase::~SharedPtrHolderBase()
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
