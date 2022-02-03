//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::ComponentIterator is a helper template class to iterate components
// inside a provided entity in order to check if it suits a desired preset.
template<int id, int depth, typename PtrContainer>
struct ComponentIterator
{
	typedef typename PtrContainer::template Item<id>::Type ItemPtr;
	typedef typename ut::RemovePointer<ItemPtr>::Type ItemType;

	static bool Suits(Entity& entity, PtrContainer& container)
	{
		ut::Optional<ItemType&> result = entity.GetComponent<ItemType>();
		if (result)
		{
			container.template Get<ItemType*>() = static_cast<ItemType*>(&result.Get());
			return ComponentIterator<id + 1, depth, PtrContainer>::Suits(entity, container);
		}
		return false;
	}
};

// Specialization of the ve::ComponentIterator for the last component.
template<int depth, typename PtrContainer>
struct ComponentIterator<depth, depth, PtrContainer>
{
	static bool Suits(Entity& entity, PtrContainer& container)
	{
		return true;
	}
};

//----------------------------------------------------------------------------//
// ve::ComponentSystem is a template class that simplifies registration of
// entities. It accepts variable number of template arguments, and each one
// represents a component that must be present inside an entity so that it
// could be registered. Therefore, only entities that have all(!) needed
// components will be registered. Registered entities are stored in the special
// array ('@entities') that holds pointers to the components and entity id.
template<typename... Components>
class ComponentSystem : public System
{
public:
	// Type of container holding pointers to all needed components.
	typedef ut::Container<Components*...> ComponentPtrSet;

	// Constructor.
	//    @param system_name - name of the system.
	ComponentSystem(ut::String system_name) : System(ut::Move(system_name))
	{}

protected:
	// Set is a proxy structure holding entity id and
	// pointers to all needed components.
	struct Set
	{
	public:
		// Constructor.
		//    @param entity_id - identifier of the registered entity.
		Set(Entity::Id entity_id) : id(entity_id)
		{}

		// Returns a reference to the component of the specified type.
		template<typename Component>
		Component& Get()
		{
			return *components.template Get<Component*>();
		}

		// id of the entity
		const Entity::Id id;

		// pointers to the components that belong
		// to the managed entity
		ComponentPtrSet components;
	};

	// Registers provided entity. And it will be registered only if
	// it has all needed components.
	//    @param id - identifier of the entity.
	//    @param entity - reference to the entity.
	//    @return - 'true' if entity was registered successfully.
	bool RegisterEntity(Entity::Id id, Entity& entity) override
	{
		// create a new proxy
		Set set(id);

		// check if entity has all components
		if (!ComponentIterator<0, ComponentPtrSet::size, ComponentPtrSet>::Suits(entity, set.components))
		{
			return false;
		}

		// add proxy to the array if it has all needed components
		if (!entities.Add(ut::Move(set)))
		{
			return false;
		}

		// success
		return true;
	}

	// registered entities
	ut::Array<Set> entities;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//