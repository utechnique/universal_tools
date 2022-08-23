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

	// Destructor destroys entities.
	~Environment();

	// Main loop.
	//    @return - optional ut::Error if environment stopped
	//              working due to internal error.
	ut::Optional<ut::Error> Run();

	// Enqueues a command, it will be executed at
	// the beginning of the next tick.
	//    @param command - unique pointer to the command to be executed.
	//    @return - optional ut::Error if failed to add provided command.
	ut::Optional<ut::Error> EnqueueCommand(ut::UniquePtr<Cmd> command);

	// Adds a new entity to the environment. This function is unsafe if
	// this environment is already running, enqueue ve::CmdAddEntity command
	// instead.
	//    @param entity - new entity object.
	//    @return - id of the entity or ut::Error if failed to
	//              add @entity to the environment.
	ut::Result<Entity::Id, ut::Error> AddEntity(Entity entity);

	// Deletes the entity from the environment. This function is unsafe if
	// this environment is already running, enqueue ve::CmdDeleteEntity command
	// instead.
	//    @param entity_id - identifier of the desired entity.
	void DeleteEntity(Entity::Id entity_id);

	// Updates desired component. This function is unsafe if
	// this environment is already running, enqueue ve::CmdUpdateComponent
	// command instead.
	//    @param entity_id - id of the entity owning the desired component.
	//    @param component_type - type of the component.
	//    @param serialized_data - reference to the json document representing
	//                             the serialized data.
	//    @param parameter_name - name of the parameter to be updated. Whole
	//                            component will be updated if this parameter
	//                            is empty.
	//    @return - optional ut::Error if failed to update the component.
	ut::Optional<ut::Error> UpdateComponent(Entity::Id entity_id,
	                                        ut::DynamicType::Handle component_type,
	                                        ut::JsonDoc& serialized_data,
	                                        const ut::Optional<ut::String>& parameter_name = ut::Optional<ut::String>());

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
	ut::ThreadPool<System::Result, ut::pool_sync::cond_var> pool;

	// Commands to be executed in the next tick.
	ut::Synchronized<CmdArray> commands;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//