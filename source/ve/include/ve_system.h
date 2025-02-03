//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
#include "ve_entity.h"
#include "ve_cmd.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::System is a base class that implements logic of interacting with
// entities and producing events. You can override ve::System::Update() function
// in a derived class to interact with registered entities. Entities are
// registered by ve::Environment with a call to ve::System::RegisterEntity()
// function (that can be overridden by derived class too).
class System
{
public:
	typedef double Time;

	// Return type of the ve::System::Update() call.
	typedef ut::Result<ve::CmdArray, ut::Error> Result;

	// Constructor.
	//    @param system_name - name of the system.
	System(ut::String system_name = ut::String("unnamed"));

	// Virtual destructor.
	virtual ~System() = default;

	// Registers provided entity.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	//    @return - 'true' if entity was registered successfully.
	virtual bool RegisterEntity(Entity::Id id, ComponentAccessGroup& access);

	// Unregisters the desired entity by its identifier.
	//    @param id - identifier of the entity.
	//    @param access - reference to the object providing access to
	//                    components.
	virtual void UnregisterEntity(Entity::Id id, ComponentAccessGroup& access);

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @param time_step_ms - time step for the current frame in milliseconds.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	virtual Result Update(Time time_step_ms, ComponentAccessGroup& access) = 0;

	// Creates component sets specific to the current system. Depending on
	// the returned component types, the @Update method will receive a
	// reference to the ve::ComponentAccessGroup object (as an argument) 
	// providing the access only for the desired components. If the returned
	// object is empty - the system will receive all component types.
	virtual ut::Array< ComponentSet<ComponentMap::Access::read_write> > DefineComponentSets() const;

	// Returns name of the system.
	const ut::String& GetName() const;

private:
	// name of the system
	ut::String name;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//