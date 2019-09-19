//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "system/ut_endianness.h"
#include "text/ut_text_node.h"
#include "meta/ut_reflective.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Forward declarations.
class Controller;

//----------------------------------------------------------------------------//
// ut::meta::BaseParameter is abstract class to serialize custom data. Every
// serializable parameter must be derived from this interface and implement
// two functions: ut::BaseParameter::Save() and ut::BaseParameter::Load().
class BaseParameter : public Reflective
{
public:
	// Constructor
	//    @param p - pointer to the serializable data
	BaseParameter(void* p);

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	virtual void Reflect(Snapshot& snapshot);

	// Returns the name of the managed type
	virtual String GetTypeName() const = 0;

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(Controller& controller);

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(Controller& controller);

	// Returns 'true' if current parameter is a container
	// for multiple uniform objects.
	virtual bool IsArray() const;

protected:
	// pointer to the managed object
	void* ptr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//