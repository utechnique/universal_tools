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
	// These operation types define how component sets (in analogy to
	// mathematic set theory) are associated with a particular entity.
	enum class EntityAssociationOperation
	{
		// Entities associated with AT LEAST ONE component from a list
		// are registered:
		unite,

		// Only entities associated with ALL components from a list
		// are registered:
		intersect
	};

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
	// Describes what type of access a component map provides for its components.
	enum class Access
	{
		// Components can only be read.
		read,

		// Components can be both read and modified.
		read_write
	};


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
template<ComponentMap::Access access = ComponentMap::Access::read>
struct SharedComponentMap
{
	typedef ut::SharedPtr<ComponentMap, ut::thread_safety::Mode::off> Type;
};

template<> struct SharedComponentMap<ComponentMap::Access::read_write>
{
	typedef ut::SharedPtr<MutableComponentMap, ut::thread_safety::Mode::off> Type;
};

// Depending on the provided template argument the ve::ComponentMapCollection
// alias template provides either full or read-only access to the managed
// component maps.
template<ComponentMap::Access access = ComponentMap::Access::read>
using ComponentMapCollection = ut::DenseHashMap<ut::DynamicType::Handle, typename SharedComponentMap<access>::Type >;

//----------------------------------------------------------------------------->
// ve::ComponentSet represents a set of entities where every entity
// is guaranteed to be associated with components from the @component_maps.
template<ComponentMap::Access access>
struct ComponentSet
{
	// This map contains hashmaps of components (one map per component type).
	ComponentMapCollection<access> component_maps;

	// The type of this set, can be 'union' or 'intersection' in analogy to the
	// mathematic set theory
	Component::EntityAssociationOperation operation = Component::EntityAssociationOperation::intersect;
};

// The same as ve::ComponentSet<ComponentMap::Access::read> but also provides an interface
// to iterate entities associated with desired components.
struct IterativeComponentSet : public ComponentSet<ComponentMap::Access::read>
                             , public ut::DenseHashMap<Entity::Id, Entity>
{
	typedef ComponentSet<ComponentMap::Access::read> Base;
	IterativeComponentSet(Base&& base) : Base(ut::Move(base)) {}
};

//----------------------------------------------------------------------------->
// ve::ComponentAccess provides a read-only access to the 
// ve::IterativeComponentSet object.
class ComponentAccess
{
public:
	// Constructor accepts the reference to the component set.
	ComponentAccess(IterativeComponentSet& in_component_set) : component_set(in_component_set)
	{}

	// Returns the reference to the desired entity.
	//    @param id - counter index of the entity (can be used with
	//                CountEntities() method). WARNING! Not the same as
	//                ve::Entity::Id!
	//    @return - const reference to the Id/Entity pair.
	const ut::Pair<const Entity::Id, Entity>& operator [] (const size_t index) const
	{
		return component_set[index];
	}

	// Returns the component map of the desired type or nothing if not found.
	template<typename ComponentType>
	inline ut::Optional<ComponentMap&> GetMap() const
	{
		ut::Optional<SharedComponentMap<ComponentMap::Access::read>::Type&> map =
			component_set.component_maps.Find(ut::GetPolymorphicHandle<ComponentType>());
		return map ? ut::Optional<ComponentMap&>(map->GetRef()) : ut::Optional<ComponentMap&>();
	}

	// Returns the component map of the desired type or nothing if not found.
	inline ut::Optional<ComponentMap&> GetMap(ut::DynamicType::Handle component_type) const
	{
		ut::Optional<SharedComponentMap<ComponentMap::Access::read>::Type&> map =
			component_set.component_maps.Find(component_type);
		return map ? ut::Optional<ComponentMap&>(map->GetRef()) : ut::Optional<ComponentMap&>();
	}

	// Returns 'true' if an entity with such id is registered.
	inline bool IsEntityRegistered(Entity::Id entity_id) const
	{
		return component_set.Find(entity_id).HasValue();
	}

	// Returns a read / write iterator that points to the first entity.
	inline IterativeComponentSet::Iterator BeginEntities() const
	{
		return component_set.Begin();
	}

	// Returns a read / write iterator that points to the last entity.
	inline IterativeComponentSet::Iterator EndEntities() const
	{
		return component_set.End();
	}

	inline ut::Optional<const Entity&> FindEntity(Entity::Id id) const
	{
		return static_cast<const IterativeComponentSet&>(component_set).Find(id);
	}

	// Returns the number of entities that can be accessed.
	inline size_t CountEntities() const
	{
		return component_set.Count();
	}

	// Returns the number of component types that can be accessed.
	inline size_t CountComponentTypes() const
	{
		return component_set.component_maps.Count();
	}

private:
	IterativeComponentSet& component_set;
};

//----------------------------------------------------------------------------->
// ve::ComponentAccess is a safe interface providing a selective access for the
// desired components maps. One can modify components inside a map, but cannot
// change the map itself.
class ComponentAccessGroup
{
	template<typename Component, typename... ComponentTail>
	struct AccessHelper
	{
		static bool CheckIfSetContainsAllAndOnlyComponents(const ComponentAccess& access, size_t index = 1)
		{
			return AccessHelper<Component>::CheckIfSetContainsAllAndOnlyComponents(access, access.CountComponentTypes()) ?
				AccessHelper<ComponentTail...>::CheckIfSetContainsAllAndOnlyComponents(access, index + 1) : false;
		}
	};

	template<typename Component>
	struct AccessHelper<Component>
	{
		static bool CheckIfSetContainsAllAndOnlyComponents(const ComponentAccess& access, size_t index = 1)
		{
			return index == access.CountComponentTypes() && static_cast<bool>(access.GetMap<Component>());
		}
	};

	// Returns the component access set providing the access to all (and only)
	// desired component types.
	template<typename AccessType, typename... Components>
	inline ut::Optional<AccessType> GetAccessImpl()
	{
		const size_t access_set_count = component_sets.Count();
		for (size_t i = 0; i < access_set_count; i++)
		{
			AccessType access_set = component_sets[i];
			if (AccessHelper<Components...>::CheckIfSetContainsAllAndOnlyComponents(access_set))
			{
				return access_set;
			}
		}

		return ut::Optional<AccessType>();
	}

public:
	// Returns the refenrece to the ve::ComponentAccess object providing an
	// access to the entities associated simultaneously with all desired
	// template component argument types (and only them) or nothing if there
	// is no such access configuration available in this group.
	template<typename... Components>
	ut::Optional<ComponentAccess> GetAccess()
	{
		const size_t component_set_count = component_sets.Count();
		for (size_t i = 0; i < component_set_count; i++)
		{
			IterativeComponentSet& component_set = component_sets[i];
			if (AccessHelper<Components...>::CheckIfSetContainsAllAndOnlyComponents(component_set))
			{
				return ComponentAccess(component_set);
			}
		}

		return ut::Optional<ComponentAccess>();
	}

	// Returns the component access set with the desired index (const).
	ComponentAccess GetAccess(size_t index)
	{
		return ComponentAccess(component_sets[index]);
	}

	// Returns the number of component combinations that can be accessed.
	inline size_t CountComponentSets() const
	{
		return component_sets.Count();
	}

protected:
	ut::Array<IterativeComponentSet> component_sets;
};

// ve::SynchronizableComponentAccess is a ve::ComponentAccess
// that can be synchronized with the external component map collection.
class SynchronizableComponentAccessGroup : public ComponentAccessGroup
{
public:
	typedef ut::Array< ComponentSet<ComponentMap::Access::read> > SyncSource;

	// Default constructor initializes child ve::ComponentAccessGroup
	// class to have access to the managed component maps.
	SynchronizableComponentAccessGroup(SyncSource source = SyncSource())
	{
		const size_t src_maps_count = source.Count();
		for (size_t i = 0; i < src_maps_count; i++)
		{
			component_sets.Add(IterativeComponentSet(ut::Move(source[i])));
		}
	}

	// Copying is prohibited.
	SynchronizableComponentAccessGroup(const SynchronizableComponentAccessGroup&) = delete;
	SynchronizableComponentAccessGroup& operator = (const SynchronizableComponentAccessGroup&) = delete;

	// Move is allowed.
	SynchronizableComponentAccessGroup(SynchronizableComponentAccessGroup&&) = default;
	SynchronizableComponentAccessGroup& operator = (SynchronizableComponentAccessGroup&&) = default;

	// Registers the provided entity if it has exactly
	// the same set of components.
	//    @param entity_id - id of the entity to be registered.
	//    @param entity - const reference to the entity.
	//    @return - 'true' if successfully registered the entity
	//              or 'false' otherwise.
	bool RegisterEntity(Entity::Id entity_id, const Entity& entity)
	{
		bool result = false;

		const size_t component_set_count = component_sets.Count();
		for (size_t i = 0; i < component_set_count; i++)
		{
			IterativeComponentSet& set = component_sets[i];
			
			if (set.operation == Component::EntityAssociationOperation::intersect)
			{
				bool has_all_components = true;

				ComponentMapCollection<ComponentMap::Access::read>::ConstIterator component_map_iterator;
				for (component_map_iterator = set.component_maps.Begin();
				     component_map_iterator != set.component_maps.End();
				     ++component_map_iterator)
				{
					if (!entity.HasComponent(component_map_iterator->GetFirst()))
					{
						has_all_components = false;
						break;
					}
				}

				if (!has_all_components)
				{
					continue;
				}
			}

			set.Insert(entity_id, entity);
			result = true;
		}

		return result;
	}

	// Unregisters the provided entity.
	//    @param entity_id - id of the entity to be unregistered.
	//    @return - 'true' if provided entity was found and unregistered.
	bool UnregisterEntity(Entity::Id entity_id)
	{
		bool result = false;
		const size_t component_set_count = component_sets.Count();
		for (size_t i = 0; i < component_set_count; i++)
		{
			const bool remove_result = component_sets[i].Remove(entity_id);
			if (!result)
			{
				result = remove_result;
			}
		}
		return result;
	}

	// Returns 'true' if an entity with such id is registered.
	inline bool IsEntityRegistered(Entity::Id entity_id) const
	{
		const size_t component_set_count = component_sets.Count();
		for (size_t i = 0; i < component_set_count; i++)
		{
			if (component_sets[i].Find(entity_id))
			{
				return true;
			}
		}
		return false;
	}

private:
	bool accept_all_entities = true;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//