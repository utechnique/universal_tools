//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commands/ve_cmd_add_entity.h"
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
CmdAddEntity::CmdAddEntity(Entity in_entity) noexcept : entity(ut::Move(in_entity))
{}

// Connects provided function with signal that is triggered after a call
// to ve::Environment::AddEntity().
void CmdAddEntity::Connect(ut::Function<void(const AddResult&)> slot)
{
	signal.Connect(ut::Move(slot));
}

// Calls ve::Environment::AddEntity().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdAddEntity::Execute(Environment& environment)
{
	ut::Result<Entity::Id, ut::Error> result = environment.AddEntity(ut::Move(entity));
	signal(result);
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//