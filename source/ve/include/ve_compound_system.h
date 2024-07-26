//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::CompoundAccessStaticIterator is a helper template class to iterate
// compound access objects in order to form a fast access interface to the
// desired component sets.
template<int id, int depth, typename CompoundAccessTuple>
struct CompoundAccessStaticIterator
{
	static void GenerateComponentSets(ut::Array< ComponentSet<ut::access_full> >& component_sets)
	{
		CompoundAccessStaticIterator<id, id, CompoundAccessTuple>::GenerateComponentSets(component_sets);
		CompoundAccessStaticIterator<id + 1, depth, CompoundAccessTuple>::GenerateComponentSets(component_sets);
	}
};

// Specialization of the ve::CompoundAccessStaticIterator for the last item.
template<int id, typename CompoundAccessTuple>
struct CompoundAccessStaticIterator<id, id, CompoundAccessTuple>
{
	static void GenerateComponentSets(ut::Array< ComponentSet<ut::access_full> >& component_sets)
	{
        typedef typename CompoundAccessTuple::template Item<id>::Type AccessType;

		component_sets.Add(ComponentSet<ut::access_full>());
		ComponentMapStaticIterator<0, AccessType::size - 1,
		                           AccessType>::GenerateComponentMaps(component_sets.GetLast().component_maps);
		component_sets.GetLast().operation = Component::op_intersection;
	}
};

//----------------------------------------------------------------------------//
// ve::CompoundAccess is a template alias for describing a group of components.
// It is expected to be used in ve::CompoundSystem as a template argument(s)
// and as an access interface for the desired component set.
template<typename... Components>
using CompoundAccess = typename ComponentSystem<Components...>::Access;

//----------------------------------------------------------------------------//
// ve::CompoundSystem is a template class that simplifies registration of
// entities that are grouped by component sets. It accepts variable number
// of template arguments, and each of them must be a ve::CompoundAccess template
// containing component types that must be present inside a desired group of
// entities. One can get access to the registered entity group by using
// ve::CompoundSystem::Access interface that is passed to the overridden
// virtual ve::CompoundSystem::Update() function.
template<typename... CompoundAccesses>
class CompoundSystem : public System
{
	typedef ut::Tuple<CompoundAccesses...> CompoundAccessTuple;
public:

	// Represents a tuple with access to all registered groups of entities.
	class Access : public CompoundAccessTuple
	{
	public:
		// Use this type to iterate registered entities.
		typedef IterativeComponentSet::Iterator EntityIterator;

		// Template argument list can't be empty.
		static_assert(CompoundAccessTuple::size > 0, "ve::CompoundSystem must have at least one template argument.");

		// Constructor initializes managed component accesses.
		Access(ComponentAccessGroup& group_access) : CompoundAccessTuple(
		// VS 2015 cannot recognize a constructor in the pack expansion here
			CompoundAccesses(group_access)...)
		{}
	};

	// Constructor.
	//    @param system_name - name of the system.
	CompoundSystem(ut::String system_name) : System(ut::Move(system_name))
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
	// Creates component maps specific to the current system.
	virtual ut::Array< ComponentSet<ut::access_full> > DefineComponentSets() const override
	{
		ut::Array< ComponentSet<ut::access_full> > sets;
		CompoundAccessStaticIterator<0, CompoundAccessTuple::size - 1,
		                             CompoundAccessTuple>::GenerateComponentSets(sets);
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