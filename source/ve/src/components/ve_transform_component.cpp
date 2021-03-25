//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "components/ve_transform_component.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::Component, ve::TransformComponent, "transform")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Identify() method must be implemented for polymorphic types.
const ut::DynamicType& TransformComponent::Identify() const
{
	return ut::Identify(this);
}

// Registers transform info into the reflection tree.
//    @param snapshot - reference to the reflection tree.
void TransformComponent::Reflect(ut::meta::Snapshot& snapshot)
{
	Transform::Reflect(snapshot);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//