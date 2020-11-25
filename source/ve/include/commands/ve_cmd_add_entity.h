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
// ve::CmdExit command tells adds an entity to the environment.
class CmdAddEntity : public Cmd
{
public:
	// Return type of the ve::Environment::AddEntity() function.
	typedef ut::Result<Entity::Id, ut::Error> AddResult;

	// Constructor.
	CmdAddEntity(Entity in_entity) noexcept;

	// Calls ve::Environment::AddEntity().
	//    @param environment - reference to the environment
	//                         executing the command.
	//    @return - optional ut::Error if environment failed to execute
	//              the command.
	ut::Optional<ut::Error> Execute(Environment& environment);

	// Connects provided function with signal that is triggered after a call
	// to ve::Environment::AddEntity().
	void Connect(ut::Function<void(const AddResult&)> slot);

private:
	// Managed entity to be added.
	Entity entity;

	// Signal that is triggered after a call to ve::Environment::AddEntity().
	ut::Signal<void(const AddResult&)> signal;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//