//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_cmd.h"
#include "ve_entity.h"
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::CmdAddEntity adds a new component to the environment.
class CmdAddComponent : public Cmd
{
public:
	// Constructor.
	//    @param entity_id - identifier of the entity to add the component.
	//    @param component - the unique pointer to the component to be added.
	CmdAddComponent(Entity::Id entity_id,
	                ut::UniquePtr<Component> component) noexcept;

	// Calls ve::Environment::AddComponent().
	//    @param environment - reference to the environment
	//                         executing the command.
	//    @return - optional ut::Error if environment failed to execute
	//              the command.
	ut::Optional<ut::Error> Execute(Environment& environment);

	// Connects provided function with signal that is triggered after a call
	// to ve::Environment::AddComponent().
	void Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot);

private:
	// Entity to add the component to.
	Entity::Id entity_id;

	// Component to be added.
	ut::UniquePtr<Component> component;

	// Signal that is triggered after a call to ve::Environment::AddEntity().
	ut::Signal<void(const ut::Optional<ut::Error>&)> signal;

};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//