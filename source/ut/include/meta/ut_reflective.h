//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Forward declarations.
class Snapshot;

//----------------------------------------------------------------------------//
// ut::Reflective is an abstract class to provide reflection of it's contents.
// Classes inherited from ut::Reflective must implement Reflect(Snapshot&)
// method to serialize(register) member parameters.
class Reflective
{
public:
	// Virtual destructor.
	virtual ~Reflective() = default;

	// Registers parameters to the reflection tree.
	virtual void Reflect(Snapshot& snapshot) = 0;

	// String with the name of ut::meta::Reflective type
	static const char* skTypeName;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//