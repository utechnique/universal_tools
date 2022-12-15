//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_render_toolset.h"

//----------------------------------------------------------------------------//
// Render units.
#include "units/ve_render_view.h"
#include "units/ve_render_model.h"
#include "units/ve_render_directional_light.h"
#include "units/ve_render_point_light.h"
#include "units/ve_render_spot_light.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// List of unit types used by render engine.
using EngineUnits = ut::Container
<
	View,
	Model,
	DirectionalLight,
	PointLight,
	SpotLight
>;

//----------------------------------------------------------------------------//
// ve::render::Policies is a template class representing a set of policies used
// by render engine.
template<class UnitContainer> class PolicyContainerTemplate;

template<class... Units>
class PolicyContainerTemplate< ut::Container<Units...> > : public ut::Container<Policy<Units>&...>
{
	typedef ut::Container<Policy<Units>&...> BaseContainer;
public:
	PolicyContainerTemplate(Policy<Units>&... policies) : BaseContainer(policies...) {}
	template<class Unit> Policy<Unit>& Get() { return BaseContainer::template Get<Policy<Unit>&>(); }
};

using Policies = PolicyContainerTemplate<EngineUnits>;

//----------------------------------------------------------------------------//
// Helps the ve::render::UnitSelector to recursively iterate unit types.
template<int id, typename ContainerType>
struct SelectorHelper
{
	inline static void Select(ContainerType& container,
	                          Entity::Id entity_id,
	                          Unit& unit,
	                          ut::Array<Entity::Id>* map)
	{
		if (!SelectorHelper<0, ContainerType>::template SelectById<id>(container, entity_id, unit, map))
		{
			SelectorHelper<id - 1, ContainerType>::Select(container, entity_id, unit, map);
		}
	}

	inline static void Remove(ContainerType& container,
	                          Entity::Id entity_id,
	                          ut::Array<Entity::Id>* map)
	{
		SelectorHelper<0, ContainerType>::template RemoveById<id>(container, entity_id, map);
		SelectorHelper<id - 1, ContainerType>::Remove(container, entity_id, map);
	}
};

// ve::render::SelectorHelper specialization for the last unit type.
template<typename ContainerType>
struct SelectorHelper<0, ContainerType>
{
	template<int type_id>
	inline static bool SelectById(ContainerType& container,
	                              Entity::Id entity_id,
	                              Unit& unit,
	                              ut::Array<Entity::Id>* map)
	{
		typedef typename EngineUnits::template Item<type_id>::Type UnitType;
		const ut::DynamicType::Handle unit_handle = ut::GetPolymorphicHandle<UnitType>();
		const ut::DynamicType& unit_type = unit.Identify();
		if (unit_type.GetHandle() != unit_handle)
		{
			return false;
		}

		ut::Array< ut::Ref<UnitType> >& dst = container.template Get<type_id>();
		dst.Add(static_cast<UnitType&>(unit));
		map[type_id].Add(entity_id);

		return true;
	}

	template<int type_id>
	inline static void RemoveById(ContainerType& container,
	                              Entity::Id entity_id,
	                              ut::Array<Entity::Id>* map)
	{
		typedef typename EngineUnits::template Item<type_id>::Type UnitType;
		ut::Array<Entity::Id>& unit_map = map[type_id];
		ut::Array< ut::Ref<UnitType> >& units = container.template Get<type_id>();
		const size_t unit_count = units.Count();
		for (size_t i = unit_count; i-- > 0; )
		{
			if (unit_map[i] == entity_id)
			{
				units.Remove(i);
				unit_map.Remove(i);
			}
		}
	}

	inline static void Select(ContainerType& container,
	                          Entity::Id entity_id,
	                          Unit& unit,
	                          ut::Array<Entity::Id>* map)
	{
		SelectById<0>(container, entity_id, unit, map);
	}

	inline static void Remove(ContainerType& container,
	                          Entity::Id entity_id,
	                          ut::Array<Entity::Id>* map)
	{
		RemoveById<0>(container, entity_id, map);
	}
};

// ve::render::UnitSelector is ut::Selector template working with render units.
template<class UnitContainer> class UnitSelectorTemplate;
template<class... Units>
class UnitSelectorTemplate< ut::Container<Units...> > : public ut::Container<ut::Array< ut::Ref<Units> >... >
{
	typedef ut::Container<ut::Array< ut::Ref<Units> >... > BaseContainer;
	static constexpr int last_unit_type_id = BaseContainer::size - 1;
public:
	// Resets all managed arrays of derived types, makes them empty.
	void Reset()
	{
		SelectorHelper<last_unit_type_id, BaseContainer>::Reset(*this, map);
	}

	// Selects objects of the @Derived types and appends them to the
	// corresponding array.
	//    @param src - reference to the array of pointers of the base type.
	template<typename UnitPtr = ut::UniquePtr<Unit> >
	inline void Select(Entity::Id entity_id, ut::Array<UnitPtr>& units)
	{
		ut::ScopeLock lock(mutex);
		const size_t unit_count = units.Count();
		for (size_t i = 0; i < unit_count; i++)
		{
			if (units[i].Get() == nullptr)
			{
				continue;
			}

			SelectorHelper<last_unit_type_id, BaseContainer>::Select(*this, entity_id, units[i].GetRef(), map);
		}
	}

	// Removes all units belonging to the entity with the specified id.
	void Remove(Entity::Id entity_id)
	{
		ut::ScopeLock lock(mutex);
		SelectorHelper<last_unit_type_id, BaseContainer>::Remove(*this, entity_id, map);
	}

	// Returns a reference to the array of references to objects of the type
	// specified by template parameter.
	template<class UnitType>
	inline ut::Array< ut::Ref<UnitType> >& Get()
	{
		return BaseContainer::template Get< ut::Array< ut::Ref<UnitType> > >();
	}

private:
	// Has the same size as the corresponding unit array and contains
	// identifiers of the parent entity.
	ut::Array<Entity::Id> map[BaseContainer::size];

	// Protects Remove() and Select() methods.
	ut::Mutex mutex;
};

using UnitSelector = UnitSelectorTemplate<EngineUnits>;

//----------------------------------------------------------------------------//
// ve::render::UnitInitializer is a template class helping unit manager
// initialize units.
template<class UnitContainer> struct UnitInitializer;

template<class FirstUnitType, class... OtherUnits>
struct UnitInitializer< ut::Container<FirstUnitType, OtherUnits...> >
{
	static void Initialize(Unit& unit, Policies& policies)
	{
		const ut::DynamicType& unit_type = unit.Identify();
		if (unit_type.GetHandle() == ut::GetPolymorphicHandle<FirstUnitType>())
		{
			policies.Get<FirstUnitType>().Initialize(static_cast<FirstUnitType&>(unit));
			unit.initialized = true;
			return;
		}
		UnitInitializer< ut::Container<OtherUnits...> >::Initialize(unit, policies);
	}
};

// Specialization for the last unit type in a list.
template<class LastUnitType>
struct UnitInitializer< ut::Container<LastUnitType> >
{
	static void Initialize(Unit& unit, Policies& policies)
	{
		const ut::DynamicType& unit_type = unit.Identify();
		if (unit_type.GetHandle() != ut::GetPolymorphicHandle<LastUnitType>())
		{
			return;
		}

		policies.Get<LastUnitType>().Initialize(static_cast<LastUnitType&>(unit));
		unit.initialized = true;
	}
};

//----------------------------------------------------------------------------//
// ve::render::UnitManager ties together units and corresponding policies.
template<class UnitContainer> class UnitManagerTemplate;

template<class... Units>
class UnitManagerTemplate< ut::Container<Units...> > : private Policy<Units>...
{
public:
	// Constructor.
	UnitManagerTemplate(Toolset &toolset_ref) : Policy<Units>(toolset_ref, selector, policies)...
                                              , tools(toolset_ref)
	                                          , policies(static_cast<Policy<Units>&>(*this)...)
	{}

	// Initializes a unit via the corresponding policy.
	void InitializeUnit(Unit& unit)
	{
		UnitInitializer< ut::Container<Units...> >::Initialize(unit, policies);
	}

	// Selector owns units and classifies them by type.
	UnitSelector selector;

	// Policy container is a set of policies, one for each unit type.
	Policies policies;

private:
	// helper tools
	Toolset &tools;
};

using UnitManager = UnitManagerTemplate<EngineUnits>;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
