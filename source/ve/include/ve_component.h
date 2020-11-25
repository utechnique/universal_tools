//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::Component is a base class for components. Each component represents
// a unique quantum of information to model behaviour of the entity. Don't
// forget to register derived classes by UT_REGISTER_TYPE() macro and to
// override ve::Component::Identify() method.
class Component : public ut::Polymorphic, public ut::meta::Reflective
{
public:
	// Identify() method must be implemented for the polymorphic types.
	virtual const ut::DynamicType& Identify() const = 0;

	// Register members here
	virtual void Reflect(ut::meta::Snapshot& snapshot);

	// ve::Component is abstract class, therefore must have virtual destructor.
	virtual ~Component() = default;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//