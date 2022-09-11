//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commands/ve_cmd_delete_entity.h"
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
CmdDeleteEntity::CmdDeleteEntity(Entity::Id id) noexcept : entity_id(id)
{}

// Calls ve::Environment::DeleteEntity().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdDeleteEntity::Execute(CmdAccessibleEnvironment& environment)
{
	signal(environment.DeleteEntity(entity_id));
	return ut::Optional<ut::Error>();
}

// Connects provided function with signal that is triggered after a call
// to ve::Environment::AddEntity().
void CmdDeleteEntity::Connect(ut::Function<void(const ut::Optional<ut::Error>&)> slot)
{
	signal.Connect(ut::Move(slot));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//