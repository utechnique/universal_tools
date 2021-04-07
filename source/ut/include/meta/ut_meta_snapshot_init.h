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
// Creates a binary parameter from the provided object. Be careful! Binary
// parameters might be potentially unsafe. Use it only for simple types that can
// be initialized just by copying its memory.
template<typename T>
BinaryParameter<T> Binary(T& object, ut::uint32 granularity = 1)
{
	return BinaryParameter<T>(&object, granularity);
}

// Builds a full reflection tree from a provided object.
//    @param object - reference to the object to be reflected.
//    @param name - name of the @object.
//    @return - ut::Error if failed.
template <typename T>
Optional<Error> Snapshot::Init(T& object, String name)
{
	// create one of the possible parameters - correct
	// variant is deduced from the argument list
	data.parameter = MakeShared< Parameter<T> >(&object);

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