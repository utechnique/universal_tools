//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_id_generator.h"
#include "ve_entity.h"
#include "ve_pipeline.h"
#include "ve_default.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::Environment is a class that contains all information about the virtual
// environment (like entities and systems), and processes events and
// interactions between entities.
class Environment : public ut::NonCopyable
{
public:
	// Constructor.
	//    @param in_pipeline - ve::Pipeline object.
	Environment(Pipeline in_pipeline = GenDefaultPipeline());

	// Main loop.
	//    @return - optional ut::Error if environment stopped
	//              working due to internal error.
	ut::Optional<ut::Error> Run();

	// Adds a new entity to the environment.
	//    @param entity - new entity object.
	//    @return - id of the entity or ut::Error if failed to
	//              add @entity to the environment.
	ut::Result<Entity::Id, ut::Error> AddEntity(Entity entity);

	// Tells the environment to exit after current tick ends.
	void Exit();

private:
	// Processes provided array of commands.
	//    @param commands - array of commands to be processed by
	//                      environment.
	//    @return - optional ut::Error if at least one command
	//              failed to be executed.
	ut::Optional<ut::Error> ExecuteCommands(CmdArray commands);

	// Array of entities, where every entity has a unique id.
	ut::Map<Entity::Id, Entity> entities;

	// Id-generator is used to generate unique identifiers for new entities.
	IdGenerator<Entity::Id> id_generator;

	// Pipeline object that controls systems' execution order. 
	Pipeline pipeline;

	// Thread-safe flag that signals when to stop the main loop.
	ut::Atomic<bool> exit;

	// Thread pool that can process multiple tasks simultaneously.
	ut::ThreadPool<System::Result> pool;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//