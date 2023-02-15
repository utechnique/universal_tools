//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_entity.h"
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
	virtual void Reflect(ut::meta::Snapshot& snapshot) {};

	// ve::Component is abstract class, therefore must have virtual destructor.
	virtual ~Component() = default;
};

//----------------------------------------------------------------------------->
// ve::ComponentMap is an interface providing access to the desired component
// map without the essential need to specify the exact component map type. The
// structure of this map cannot be changed (insert/remove methods are unavailable).
class ComponentMap
{
public:
	// Finds a component by associated entity id.
	//    @param entity_id - entity identifier.
	//    @return - optional reference to the desired component.
	virtual ut::Optional<Component&> Find(Entity::Id entity_id) = 0;

	// Finds a component by associated entity id.
	//    @param entity_id - entity identifier.
	//    @return - optional constant reference to the desired component.
	virtual ut::Optional<const Component&> Find(Entity::Id entity_id) const = 0;

	// Returns the number of components in the map.
	virtual size_t Count() const = 0;

	// Returns the type handle of the managed component type.
	virtual ut::DynamicType::Handle GetComponentTypeHandle() const = 0;
};

// ve::MutableComponentMap is an interface providing access to the desired
// component map without the essential need to specify the exact component map
// type.The structure of this map is allowed be changed.
class MutableComponentMap : public ComponentMap
{
public:
	// Inserts a new component to the map.
	//    @param entity_id - entity identifier.
	//    @param component - r-value refenrence to the unique component pointer.
	//    @return - optional component reference.
	virtual ut::Optional<Component&> Insert(Entity::Id entity_id,
	                                        ut::UniquePtr<Component> component) = 0;

	// Removes the desired component.
	//    @param entity_id - entity identifier.
	//    @return - 'true' if the component was found and deleted
	//              or 'false' if there is no such component in the map.
	virtual bool Remove(Entity::Id entity_id) = 0;
};

// The implementation of the ve::ComponentMap for the desired component type.
template<typename ManagedComponentType>
class ComponentMapImpl : public MutableComponentMap
{
public:
	typedef ManagedComponentType ComponentType;
	static_assert(ut::IsBaseOf<Component, ComponentType>::value,
	              "Template argument type must be inherited from the ve::Component.");

	// Inserts a new component to the map.
	//    @param entity_id - entity identifier.
	//    @param component - r-value refenrence to the unique component pointer.
	//    @return - optional component reference.
	ut::Optional<Component&> Insert(Entity::Id entity_id,
	                                ut::UniquePtr<Component> component) override
	{
		UT_ASSERT(component->Identify().GetHandle() == ut::GetPolymorphicHandle<ComponentType>());

		ut::Optional<ut::Pair<const Entity::Id, ComponentType>&> result = map.Insert(entity_id,
		                                                                             static_cast<ComponentType&&>(ut::Move(component.GetRef())));
		if (result)
		{
			return static_cast<Component&>(result->second);
		}

		return ut::Optional<Component&>();
	}

	// Finds a component by associated entity id.
	//    @param entity_id - entity identifier.
	//    @return - optional reference to the desired component.
	ut::Optional<Component&> Find(Entity::Id entity_id) override
	{
		ut::Optional<ComponentType&> component = map.Find(entity_id);
		if (component)
		{
			return component.Get();
		}

		return ut::Optional<Component&>();
	}

	// Finds a component by associated entity id.
	//    @param entity_id - entity identifier.
	//    @return - optional constant reference to the desired component.
	ut::Optional<const Component&> Find(Entity::Id entity_id) const override
	{
		ut::Optional<const ComponentType&> component = map.Find(entity_id);
		if (component)
		{
			return component.Get();
		}

		return ut::Optional<const Component&>();
	}

	// Removes the desired component.
	//    @param entity_id - entity identifier.
	//    @return - 'true' if the component was found and deleted
	//              or 'false' if there is no such component in the map.
	bool Remove(Entity::Id entity_id) override
	{
		return map.Remove(entity_id);
	}

	// Returns the number of components in the map.
	size_t Count() const override
	{
		return map.Count();
	}

	// Returns the type handle of the managed component type.
	ut::DynamicType::Handle GetComponentTypeHandle() const
	{
		return ut::GetPolymorphicHandle<ComponentType>();
	}

private:
	ut::DenseHashMap<Entity::Id, ComponentType> map;
};

//----------------------------------------------------------------------------->
// Helper template classes for the ve::ComponentMapCollection alias template.
template<ut::Access access = ut::access_read>
struct SharedComponentMap { typedef ut::SharedPtr<ComponentMap, ut::thread_safety::off> Type; };
template<> struct SharedComponentMap<ut::access_full> { typedef ut::SharedPtr<MutableComponentMap, ut::thread_safety::off> Type; };

// Depending on the provided template argument the ve::ComponentMapCollection
// alias template provides either full or read-only access to the managed
// component maps.
template<ut::Access access = ut::access_read>
using ComponentMapCollection = ut::DenseHashMap<ut::DynamicType::Handle, typename SharedComponentMap<access>::Type >;

//----------------------------------------------------------------------------->
// ve::ComponentAccess is a safe interface providing a selective access for the
// desired components maps. One can modify components inside a map, but cannot
// change the map itself.
class ComponentAccess
{
public:
	typedef ut::DenseHashMap<Entity::Id, Entity> EntityMap;
	typedef EntityMap::ConstIterator EntityIterator;

	// Returns the reference to the desired entity.
	//    @param id - counter index of the entity (can be used with
	//                CountEntities() method). WARNING! Not the same as
	//                ve::Entity::Id!
	//    @return - const reference to the Id/Entity pair.
	const ut::Pair<const Entity::Id, Entity>& operator [] (const size_t index) const
	{
		return entities[index];
	}

	// Returns the component map of the desired type or nothing if not found.
	template<typename ComponentType>
	inline ut::Optional<ComponentMap&> GetMap()
	{
		ut::Optional<SharedComponentMap<ut::access_read>::Type&> map = component_maps.Find(ut::GetPolymorphicHandle<ComponentType>());
		return map ? ut::Optional<ComponentMap&>(map->GetRef()) : ut::Optional<ComponentMap&>();
	}

	// Returns the component map of the desired type or nothing if not found.
	template<typename ComponentType>
	inline ut::Optional<const ComponentMap&> GetMap() const
	{
		ut::Optional<const SharedComponentMap<ut::access_read>::Type&> map = component_maps.Find(ut::GetPolymorphicHandle<ComponentType>());
		return map ? ut::Optional<const ComponentMap&>(map->GetRef()) : ut::Optional<const ComponentMap&>();
	}

	// Returns the component map of the desired type or nothing if not found.
	inline ut::Optional<ComponentMap&> GetMap(ut::DynamicType::Handle component_type)
	{
		ut::Optional<SharedComponentMap<ut::access_read>::Type&> map = component_maps.Find(component_type);
		return map ? ut::Optional<ComponentMap&>(map->GetRef()) : ut::Optional<ComponentMap&>();
	}

	// Returns the component map of the desired type or nothing if not found.
	inline ut::Optional<const ComponentMap&> GetMap(ut::DynamicType::Handle component_type) const
	{
		ut::Optional<const SharedComponentMap<ut::access_read>::Type&> map = component_maps.Find(component_type);
		return map ? ut::Optional<const ComponentMap&>(map->GetRef()) : ut::Optional<const ComponentMap&>();
	}

	// Returns 'true' if an entity with such id is registered.
	inline bool IsEntityRegistered(Entity::Id entity_id) const
	{
		return entities.Find(entity_id).HasValue();
	}

	// Returns a read / write iterator that points to the first entity.
	inline EntityIterator BeginEntities() const
	{
		return entities.Begin();
	}

	// Returns a read / write iterator that points to the last entity.
	inline EntityIterator EndEntities() const
	{
		return entities.End();
	}

	inline ut::Optional<const Entity&> FindEntity(Entity::Id id) const
	{
		return entities.Find(id);
	}

	// Returns the number of entities that can be accessed.
	inline size_t CountEntities() const
	{
		return entities.Count();
	}

protected:
	// This map contains hashmaps of components (one map per component type).
	ComponentMapCollection<ut::access_read> component_maps;

	// Entity ID hashmap.
	EntityMap entities;
};

// ve::SynchronizableComponentAccess is a ve::ComponentAccess
// that can be synchronized with the external component map collection.
class SynchronizableComponentAccess : public ComponentAccess
{
public:
	// Synchronizes internal component maps with the provided collection.
	//    @param source - shared component map collection.
	//    @param accept_all - set this parameter to 'true' if you want to
	//                        accept all entities.
	void Sync(ComponentMapCollection<ut::access_read> source, bool accept_all = false)
	{
		component_maps = ut::Move(source);
		accept_all_entities = accept_all;
	}

	// Registers the provided entity if it has exactly
	// the same set of components.
	//    @param entity_id - id of the entity to be registered.
	//    @param entity - const reference to the entity.
	//    @return - 'true' if successfully registered the entity
	//              or 'false' otherwise.
	bool RegisterEntity(Entity::Id entity_id, const Entity& entity)
	{
		if (!accept_all_entities)
		{
			if (component_maps.Count() == 0)
			{
				return false;
			}

			ComponentMapCollection<ut::access_read>::ConstIterator component_map_iterator;
			for (component_map_iterator = component_maps.Begin(); component_map_iterator != component_maps.End(); ++component_map_iterator)
			{
				if (!entity.HasComponent(component_map_iterator->GetFirst()))
				{
					return false;
				}
			}
		}

		entities.Insert(entity_id, entity);
		return true;
	}

	// Unregisters the provided entity.
	//    @param entity_id - id of the entity to be unregistered.
	//    @return - 'true' if successfully unregistered the entity
	//              or 'false' if there was no entity with such id.
	bool UnregisterEntity(Entity::Id entity_id)
	{
		return entities.Remove(entity_id);
	}

private:
	bool accept_all_entities = true;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//