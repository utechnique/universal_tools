//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::ComponentMapStaticIterator is a helper template class to iterate
// component maps in order to form a fast component access interface for the
// desired component system.
template<int id, int depth, typename PtrContainer>
struct ComponentMapStaticIterator
{
	static void Initialize(ComponentAccess& access, PtrContainer& container)
	{
		ComponentMapStaticIterator<id, id, PtrContainer>::Initialize(access, container);
		ComponentMapStaticIterator<id + 1, depth, PtrContainer>::Initialize(access, container);
	}

	static void GenerateComponentMaps(ComponentMapCollection<ut::access_full>& component_maps)
	{
		ComponentMapStaticIterator<id, id, PtrContainer>::GenerateComponentMaps(component_maps);
		ComponentMapStaticIterator<id + 1, depth, PtrContainer>::GenerateComponentMaps(component_maps);
	}
};

// Specialization of the ve::ComponentMapStaticIterator for the last item.
template<int id, typename PtrContainer>
struct ComponentMapStaticIterator<id, id, PtrContainer>
{
	typedef typename PtrContainer::template Item<id>::Type MapPtrType;
	typedef typename ut::RemovePointer<MapPtrType>::Type MapType;
	typedef typename MapType::ComponentType ComponentType;
	static_assert(ut::IsBaseOf<Component, ComponentType>::value,
		"ve::ComponentSystem can only operate with template argument types "
		"inherited from the ve::Component class.");

	static void Initialize(ComponentAccess& access, PtrContainer& container)
	{
		ut::Optional<ComponentMap&> map = access.GetMap<ComponentType>();
		if (!map)
		{
			throw ut::Error(ut::error::not_supported);
		}

		container.template Get<id>() = static_cast<ComponentMapImpl<ComponentType>*>(&map.Get());
	}

	static void GenerateComponentMaps(ComponentMapCollection<ut::access_full>& component_maps)
	{
		component_maps.Insert(ut::GetPolymorphicHandle<ComponentType>(), ut::MakeUnsafeShared< ComponentMapImpl<ComponentType> >());
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
	typedef ut::Container<ComponentMapImpl<Components>*...> ComponentContainer;
public:
	// Constructor.
	//    @param system_name - name of the system.
	ComponentSystem(ut::String system_name) : System(ut::Move(system_name))
	{}

	class Access : private ComponentContainer
	{
	public:
		typedef ComponentAccess::EntityIterator EntityIterator;

		// Template argument list can't be empty.
		static_assert(ComponentContainer::size > 0, "ve::ComponentSystem must have at least one template argument.");

		const ut::Pair<const Entity::Id, Entity>& operator [] (const size_t id) const
		{
			return access[id];
		}

		// Returns the reference to the desired entity.
		//    @param id - counter index of the entity (can be used with
		//                CountEntities() method). WARNING! Not the same as
		//                ve::Entity::Id!
		//    @return - const reference to the Id/Entity pair.
		Access(ComponentAccess& component_access) : access(component_access)
		{
			ComponentMapStaticIterator<0, ComponentContainer::size - 1, ComponentContainer>::Initialize(access, *this);
		}

		template<typename ComponentType>
		inline ComponentType& GetComponent(Entity::Id entity_id)
		{
			ut::Optional<Component&> component = ComponentContainer::template Get<ComponentMapImpl<ComponentType>*>()->Find(entity_id);
			UT_ASSERT(component.HasValue());
			return static_cast<ComponentType&>(component.Get());
		}

		// Returns a read / write iterator that points to the first entity.
		inline EntityIterator BeginEntities() const
		{
			return access.BeginEntities();
		}

		// Returns a read / write iterator that points to the last entity.
		inline EntityIterator EndEntities() const
		{
			return access.EndEntities();
		}

		// Returns the number of accessible entities.
		inline size_t CountEntities() const
		{
			return access.CountEntities();
		}

	private:
		ComponentAccess& access;
	};

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	virtual System::Result Update(Access& access) = 0;

	// Registers provided entity.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	//    @return - 'true' if entity was registered successfully.
	virtual bool RegisterEntity(Entity::Id id, Access& access)
	{
		return true;
	};

	// Unregisters the desired entity by its identifier.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	virtual void UnregisterEntity(Entity::Id id, Access& access)
	{};

protected:
	// Creates component maps specific to the current system. Depending on
	// the returned types of components, the @Update method will receive a
	// reference to the ve::ComponentAccess object (as an argument) providing
	// the access only for the desired components. If the returned object is
	// empty - the system will receive all component types.
	virtual ut::Optional< ComponentMapCollection<ut::access_full> > SynchronizeComponents() const
	{
		ComponentMapCollection<ut::access_full> component_maps;
		ComponentMapStaticIterator<0, ComponentContainer::size - 1, ComponentContainer>::GenerateComponentMaps(component_maps);
		return component_maps;
	}

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update(ComponentAccess& component_access) override
	{
		Access access(component_access);
		return Update(access);
	}

	// Registers provided entity.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	//    @return - 'true' if entity was registered successfully.
	bool RegisterEntity(Entity::Id id, ComponentAccess& component_access) override
	{
		Access access(component_access);
		return RegisterEntity(id, access);
	}

	// Unregisters the desired entity by its identifier.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	void UnregisterEntity(Entity::Id id, ComponentAccess& component_access) override
	{
		Access access(component_access);
		return UnregisterEntity(id, access);
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//