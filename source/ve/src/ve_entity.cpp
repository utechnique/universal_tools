//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_entity.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Removes information about the desired component.
//    @param component_type - handle of the component type.
void Entity::RemoveComponent(ut::DynamicType::Handle component_type)
{
	for (size_t i = 0; i < Count(); i++)
	{
		if (this->operator[](i) == component_type)
		{
			Base::Remove(i);
		}
	}
}

// Returns true if this entity has the desired component.
bool Entity::HasComponent(ut::DynamicType::Handle component_type) const
{
	for (size_t i = 0; i < Count(); i++)
	{
		if (this->operator[](i) == component_type)
		{
			return true;
		}
	}
	return false;
}
//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//