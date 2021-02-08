//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Name component contains a name of the entity.
class NameComponent : public Component
{
public:
	// Constructors and move assignment operator.
	NameComponent(ut::String entity_name = "unnamed");
	NameComponent(NameComponent&&) = default;
	NameComponent& operator =(NameComponent&&) = default;

	// Copying is prohibited.
	NameComponent(const NameComponent&) = delete;
	NameComponent& operator =(const NameComponent&) = delete;

	// Meta routine.
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);
	
	ut::String name;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//