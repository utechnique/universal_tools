//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::Entity is a class representing indivisible entity containing different
// components that model it's behaviour.
class Entity : private ut::Array<ut::DynamicType::Handle>
{
	typedef ut::Array<ut::DynamicType::Handle> Base;
public:
	// Returns the reference to the desired entity.
	//    @param index - counter index of the entity (can be used with
	//                   CountEntities() method). WARNING! Not the same as
	//                   ve::Entity::Id!
	//    @return - handle of the desired component type.
	inline ut::DynamicType::Handle GetComponentByIndex(size_t index) const
	{
		return Base::operator[](index);
	}

	// Returns the number of components in this entity.
	inline size_t CountComponents() const
	{
		return Base::Count();
	}

	// Adds information about the new component belonging to this entity.
	//    @param component_type - handle of the component type.
	//    @return - 'true' if
	inline void AddComponent(ut::DynamicType::Handle component_type)
	{
		if (!Base::Add(component_type))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	// Removes information about the desired component.
	//    @param component_type - handle of the component type.
	inline void RemoveComponentByIndex(size_t index)
	{
		Base::Remove(index);
	}

	// Removes information about the desired component.
	//    @param component_type - handle of the component type.
	void RemoveComponent(ut::DynamicType::Handle component_type);

	// Returns true if this entity has the desired component.
	bool HasComponent(ut::DynamicType::Handle component_type) const;

	// Type of the identifier that can be associated with entity.
	typedef ut::uint32 Id;
};
//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//