//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_cmd.h"
#include "ve_entity.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::CmdDeleteComponent command deletes the component.
class CmdDeleteComponent : public Cmd
{
public:
	// Constructor.
	CmdDeleteComponent(Entity::Id id,
	                   ut::DynamicType::Handle type_handle) noexcept;

	// Calls ve::Environment::DeleteEntity().
	//    @param environment - reference to the environment
	//                         executing the command.
	//    @return - optional ut::Error if environment failed to execute
	//              the command.
	ut::Optional<ut::Error> Execute(Environment& environment);

	// Connects provided function with signal that is triggered after a call
	// to ve::Environment::AddEntity().
	void Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot);

private:
	// Managed entity component to be added to.
	Entity::Id entity_id;

	// Type of the component to be deleted.
	ut::DynamicType::Handle component_type;

	// Signal that is triggered after a call to ve::Environment::AddEntity().
	ut::Signal<void(const ut::Optional<ut::Error>&)> signal;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//