//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_base_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Constructor
//    @param p - pointer to the serializable data
BaseParameter::BaseParameter(void* p) : ptr(p)
{
	UT_ASSERT(p != nullptr);
}

// Registers children into reflection tree.
//    @param snapshot - reference to the reflection tree
void BaseParameter::Reflect(Snapshot& snapshot)
{ }

// Serializes managed object.
//    @param controller - meta controller that helps to write data
//    @return - ut::Error if encountered an error
Optional<Error> BaseParameter::Save(Controller& controller)
{
	return Optional<Error>();
}

// Deserializes managed object.
//    @param controller - meta controller that helps to read data
//    @return - ut::Error if encountered an error
Optional<Error> BaseParameter::Load(Controller& controller)
{
	return Optional<Error>();
}

// Links internal value with provided parameter.
// Every parameter calling Controller::WriteLink() and
// Controller::ReadLink() must override this function so that
// linker could operate with it.
//    @param parameter - parameter to link with.
//    @return - ut::Error if encountered an error
Optional<Error> BaseParameter::Link(BaseParameter& parameter)
{
	return Error(error::not_supported);
}

// Returns 'true' if current parameter is a container
// for multiple uniform objects.
bool BaseParameter::IsArray() const
{
	return false;
}

// Returns an address of the managed object.
void* BaseParameter::GetAddress()
{
	return ptr;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//