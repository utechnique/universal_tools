//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
//    @param system_name - name of the system.
System::System(ut::String system_name) : name(ut::Move(system_name))
{}

//----------------------------------------------------------------------------->
// Registers provided entity.
//    @param id - identifier of the entity.
//    @param entity - reference to the entity.
//    @return - 'true' if entity was registered successfully.
bool System::RegisterEntity(Entity::Id id, Entity& entity)
{
	return false;
};

//----------------------------------------------------------------------------->
// Updates system. This function is called once per tick
// by ve::Environment.
//    @return - array of commands to be executed by owning environment,
//              or ut::Error if system encountered fatal error.
System::Result System::Update()
{
	CmdArray empty;
	return empty;
}

//----------------------------------------------------------------------------->
// Returns name of the system.
const ut::String& System::GetName() const
{
	return name;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//