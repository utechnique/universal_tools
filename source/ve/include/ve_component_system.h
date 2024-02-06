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
template<int id, int depth, typename PtrTuple>
struct ComponentMapStaticIterator
{
	static void Initialize(ComponentAccess& access, PtrTuple& tuple)
	{
		ComponentMapStaticIterator<id, id, PtrTuple>::Initialize(access, tuple);
		ComponentMapStaticIterator<id + 1, depth, PtrTuple>::Initialize(access, tuple);
	}

	static void GenerateComponentMaps(ComponentMapCollection<ut::access_full>& component_maps)
	{
		ComponentMapStaticIterator<id, id, PtrTuple>::GenerateComponentMaps(component_maps);
		ComponentMapStaticIterator<id + 1, depth, PtrTuple>::GenerateComponentMaps(component_maps);
	}
};

// Specialization of the ve::ComponentMapStaticIterator for the last item.
template<int id, typename PtrTuple>
struct ComponentMapStaticIterator<id, id, PtrTuple>
{
	typedef typename PtrTuple::template Item<id>::Type MapPtrType;
	typedef typename ut::RemovePointer<MapPtrType>::Type MapType;
	typedef typename MapType::ComponentType ComponentType;
	static_assert(ut::IsBaseOf<Component, ComponentType>::value,
		"ve::ComponentSystem can only operate with template argument types "
		"inherited from the ve::Component class.");

	static void Initialize(ComponentAccess& access, PtrTuple& tuple)
	{
		ut::Optional<ComponentMap&> map = access.GetMap<ComponentType>();
		if (!map)
		{
			throw ut::Error(ut::error::not_supported);
		}

		tuple.template Get<id>() = static_cast<ComponentMapImpl<ComponentType>*>(&map.Get());
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
// components will be registered. One can get access to the registered entities
// and components by using ve::ComponentSystem::Access interface that is passed
// to the overridden virtual ve::ComponentSystem::Update() function.
template<typename... Components>
class ComponentSystem : public System
{
	typedef ut::Tuple<ComponentMapImpl<Components>*...> ComponentTuple;
public:

	// ve::ComponentSystem::Access is a wrapper class around the
	// ve::ComponentAccess object.
	class Access : private ComponentTuple
	{
        template <int, int, typename> friend struct ComponentMapStaticIterator;
		template <int, int, typename> friend struct CompoundAccessStaticIterator;

		// This function helps to extract desired component access from the
		// provided access group.
		static ComponentAccess GetComponentAccessFromGroup(ComponentAccessGroup& group_access)
		{
			ut::Optional<ComponentAccess> component_access = group_access.GetAccess<Components...>();
			UT_ASSERT(static_cast<bool>(component_access));
			return component_access.Move();
		}

	public:
		// Use this type to iterate registered entities.
		typedef IterativeComponentSet::Iterator EntityIterator;

		// Template argument list can't be empty.
		static_assert(ComponentTuple::size > 0, "ve::ComponentSystem must have at least one template argument.");

		// Constructor initializes managed component access.
		Access(ComponentAccessGroup& group_access) : access(GetComponentAccessFromGroup(group_access))
		{
			ComponentMapStaticIterator<0, ComponentTuple::size - 1, ComponentTuple>::Initialize(access, *this);
		}

		template<typename ComponentType>
		inline ComponentType& GetComponent(Entity::Id entity_id)
		{
			ut::Optional<Component&> component = ComponentTuple::template Get<ComponentMapImpl<ComponentType>*>()->Find(entity_id);
			UT_ASSERT(component.HasValue());
			return static_cast<ComponentType&>(component.Get());
		}

		// Returns a read / write iterator that points to the first entity.
		inline IterativeComponentSet::Iterator BeginEntities() const
		{
			return access.BeginEntities();
		}

		// Returns a read / write iterator that points to the last entity.
		inline IterativeComponentSet::Iterator EndEntities() const
		{
			return access.EndEntities();
		}

		// Range-based 'for' loop support
		inline auto begin() const -> decltype(BeginEntities()) { return BeginEntities(); }
		inline auto end() const -> decltype(EndEntities()) { return EndEntities(); }

		// Returns the number of accessible entities.
		inline size_t CountEntities() const
		{
			return access.CountEntities();
		}

		// Returns the desired entity. Call CountEntities() function
		// to be able to iterate entities using their index numbers.
		//    @param id - index of the desired entity.
		const ut::Pair<const Entity::Id, Entity>& operator [] (const size_t id) const
		{
			return access[id];
		}

	private:
		ComponentAccess access;
	};

	// Constructor.
	//    @param system_name - name of the system.
	ComponentSystem(ut::String system_name) : System(ut::Move(system_name))
	{}

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @param time_step_ms - time step for the current frame in milliseconds.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	virtual System::Result Update(System::Time time_step_ms,
	                              Access& access) = 0;

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
	// the access only for the desired components.
	virtual ut::Array< ComponentSet<ut::access_full> > DefineComponentSets() const override
	{
		ut::Array< ComponentSet<ut::access_full> > sets(1);
		sets.GetFirst().operation = Component::op_intersection;
		ComponentMapStaticIterator<0, ComponentTuple::size - 1, ComponentTuple>::GenerateComponentMaps(sets.GetFirst().component_maps);
		return sets;
	}

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @param time_step_ms - time step for the current frame in milliseconds.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update(System::Time time_step_ms,
	                      ComponentAccessGroup& group_access) override
	{
		Access access(group_access);
		return Update(time_step_ms, access);
	}

	// Registers provided entity.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	//    @return - 'true' if entity was registered successfully.
	bool RegisterEntity(Entity::Id id, ComponentAccessGroup& group_access) override
	{
		Access access(group_access);
		return RegisterEntity(id, access);
	}

	// Unregisters the desired entity by its identifier.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	void UnregisterEntity(Entity::Id id, ComponentAccessGroup& group_access) override
	{
		Access access(group_access);
		return UnregisterEntity(id, access);
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//