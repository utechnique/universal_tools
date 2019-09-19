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

// Returns 'true' if current parameter is a container
// for multiple uniform objects.
bool BaseParameter::IsArray() const
{
	return false;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//