//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_meta_snapshot.h"
#include "meta/parameters/ut_parameter_specializations.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Builds a full reflection tree from a provided object.
//    @param object - reference to the object to be reflected.
//    @param name - name of the @object.
//    @return - ut::Error if failed.
template <typename T>
Optional<Error> Snapshot::Init(T& object, String name)
{
	// create one of the possible parameters - correct
	// variant is deduced from the argument list
	data.parameter = new Parameter<T>(&object);

	// copy parameter name
	data.name = Move(name);

	// generate id
	data.id = 0;

	// register child nodes
	data.parameter->Reflect(*this);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//