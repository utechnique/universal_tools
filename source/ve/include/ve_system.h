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
	// Return type of the ve::System::Update() call.
	typedef ut::Result<ve::CmdArray, ut::Error> Result;

	// Constructor.
	//    @param system_name - name of the system.
	System(ut::String system_name = ut::String("unnamed"));

	// Virtual destructor.
	virtual ~System() = default;

	// Registers provided entity.
	//    @param id - identifier of the entity.
	//    @param entity - reference to the entity.
	//    @return - 'true' if entity was registered successfully.
	virtual bool RegisterEntity(Entity::Id id, Entity& entity);

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	virtual Result Update();

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