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

// Registers provided entity.
//    @param id - identifier of the entity.
//    @param access - reference to the object providing access to
//                    components.
//    @return - 'true' if entity was registered successfully.
bool System::RegisterEntity(Entity::Id id, ComponentAccessGroup& access)
{
	return true;
};

// Unregisters the desired entity by its identifier.
//    @param id - identifier of the entity.
//    @param access - reference to the object providing access to
//                    components.
void System::UnregisterEntity(Entity::Id id, ComponentAccessGroup& access)
{}

// Updates system. This function is called once per tick
// by ve::Environment.
//    @param time_step_ms - time step for the current frame in milliseconds.
//    @param access - reference to the object providing access to the
//                    desired components.
//    @return - array of commands to be executed by owning environment,
//              or ut::Error if system encountered fatal error.
System::Result System::Update(Time time_step, ComponentAccessGroup& access)
{
	System::Result empty;
	return empty;
}

// Creates component maps specific to the current system. Depending on
// the returned types of components, the @Update method will receive a
// reference to the ve::ComponentAccess object (as an argument) providing
// the access only for the desired components. If the returned object is
// empty - the system will receive all component types.
ut::Array< ComponentSet<ut::access_full> > System::DefineComponentSets() const
{
	return ut::Array< ComponentSet<ut::access_full> >();
}

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