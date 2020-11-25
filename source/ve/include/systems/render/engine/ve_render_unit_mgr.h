//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_render_toolset.h"

//----------------------------------------------------------------------------//
// Render units.
#include "systems/render/units/ve_render_view.h"
#include "systems/render/units/ve_render_mesh.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// List of unit types used by render engine.
using EngineUnits = ut::Container
<
	View,
	Mesh
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
// ve::render::UnitSelector is ut::Selector template working with render units.
template<class UnitContainer> class UnitSelectorTemplate;
template<class... Units>
class UnitSelectorTemplate< ut::Container<Units...> > : public ut::meta::Selector<Unit, Units...>
{ };

using UnitSelector = UnitSelectorTemplate<EngineUnits>;

//----------------------------------------------------------------------------//
// ve::render::UnitInitializer is a template class helping unit manager
// initialize units.
template<class UnitContainer> struct UnitInitializer;

template<class FirstUnitType, class... OtherUnits>
struct UnitInitializer< ut::Container<FirstUnitType, OtherUnits...> >
{
	static void Initialize(UnitSelector& selector, Policies& policies)
	{
		UnitInitializer< ut::Container<FirstUnitType> >::Initialize(selector, policies);
		UnitInitializer< ut::Container<OtherUnits...> >::Initialize(selector, policies);
	}
};

// Specialization for the last unit type in a list.
template<class LastUnitType>
struct UnitInitializer< ut::Container<LastUnitType> >
{
	static void Initialize(UnitSelector& selector, Policies& policies)
	{
		ut::Array< ut::Ref<LastUnitType> >& units = selector.Get<LastUnitType>();
		const size_t unit_count = units.GetNum();
		for (size_t i = 0; i < unit_count; i++)
		{
			LastUnitType& unit = units[i];
			if (unit.initialized)
			{
				continue;
			}

			policies.Get<LastUnitType>().Initialize(unit);
			unit.initialized = true;
		}
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
	UnitManagerTemplate(Toolset &toolset_ref) noexcept : Policy<Units>(toolset_ref, selector, policies)...
                                                       , tools(toolset_ref)
	                                                   , policies(reinterpret_cast<Policy<Units>&>(*this)...)
	{}

	// Every policy initializes units of it's type.
	void InitializeUnits()
	{
		UnitInitializer< ut::Container<Units...> >::Initialize(selector, policies);
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
