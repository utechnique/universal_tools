//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
#include "text/ut_string.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Forward declarations.
class Controller;

//----------------------------------------------------------------------------//
// ut::meta::SharedPtrHolderBase is an abstract class that is a type-erasing
// wrapper for shared pointers. One can save or load a parameter that is
// contained in the managed smart pointer without knowing exact type of that
// parameter.
class SharedPtrHolderBase
{
public:
	// Constructor
	//    @param in_address - address of the managed ut::SharedPtr object.
	SharedPtrHolderBase(void* in_address);

	// Virtual destructor.
	virtual ~SharedPtrHolderBase() = default;

	// Returns address of the managed ut::SharedPtr object.
	void* GetAddress();

	// Serializes managed object using provided controller.
	//    @param controller - reference to the controller to save the
	//                        managed object.
	//    @param name - name of the parameter.
	//    @return - ut::Error if failed.
	virtual Optional<Error> Save(Controller& controller, const String& name) = 0;

	// Deserializes managed object using provided controller.
	//    @param controller - reference to the controller to load the
	//                        managed object.
	//    @param name - name of the parameter.
	//    @return - ut::Error if failed.
	virtual Optional<Error> Load(Controller& controller,
	                             const String& name) = 0;

protected:
	// Address of the managed ut::SharedPtr object.
	void* address;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//