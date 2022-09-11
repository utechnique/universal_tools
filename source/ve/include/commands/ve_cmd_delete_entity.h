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
// ve::CmdDeleteEntity command deletes the entity from the environment.
class CmdDeleteEntity : public Cmd
{
public:
	// Constructor.
	CmdDeleteEntity(Entity::Id id) noexcept;

	// Calls ve::Environment::DeleteEntity().
	//    @param environment - reference to the environment
	//                         executing the command.
	//    @return - optional ut::Error if environment failed to execute
	//              the command.
	ut::Optional<ut::Error> Execute(CmdAccessibleEnvironment& environment);

	// Connects provided function with signal that is triggered after a call
	// to ve::Environment::AddEntity().
	void Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot);

private:
	// Managed entity to be added.
	Entity::Id entity_id;

	// Signal that is triggered after a call to ve::Environment::AddEntity().
	ut::Signal<void(const ut::Optional<ut::Error>&)> signal;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//