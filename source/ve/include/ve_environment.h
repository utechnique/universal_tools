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
// ve::CmdAccessibleEnvironment provides the encapsulation mechanism for the
// functions accessible only by commands. ve::Environment inherits this class
// using 'private' modifier, so the user can't call any of these functions
// directly (but only scheduling a command).
class CmdAccessibleEnvironment
{
public:
	// Adds a new entity to the environment.
	//    @param entity - new entity object.
	//    @return - id of the entity or ut::Error if failed to
	//              add @entity to the environment.
	virtual ut::Result<Entity::Id, ut::Error> AddEntity(Entity entity) = 0;

	// Adds a new component to the desired entity.
	//    @param entity_id - identifier of the entity to add the component.
	//    @param component - the unique pointer to the component to be added.
	//    @return - optional ut::Error if failed.
	virtual ut::Optional<ut::Error> AddComponent(Entity::Id entity_id,
	                                             ut::UniquePtr<Component> component) = 0;

	// Deletes the entity from the environment.
	//    @param entity_id - identifier of the desired entity.
	//    @return - optional ut::Error if failed.
	virtual ut::Optional<ut::Error> DeleteEntity(Entity::Id entity_id) = 0;

	// Deletes a component from the desired entity.
	//    @param entity_id - identifier of the desired entity.
	//    @param component_type - handle of the component type.
	//    @return - optional ut::Error if failed.
	virtual ut::Optional<ut::Error> DeleteComponent(Entity::Id entity_id,
	                                                ut::DynamicType::Handle component_type) = 0;

	// Updates desired component.
	//    @param entity_id - id of the entity owning the desired component.
	//    @param component_type - type of the component.
	//    @param callback - reference to the callback that accepts a reference
	//                      to the meta-snapshot of the desired parameter.
	//    @param parameter_name - name of the parameter to be updated.
	//    @return - optional ut::Error if failed to update the component.
	virtual ut::Optional<ut::Error> UpdateComponent(Entity::Id entity_id,
	                                                ut::DynamicType::Handle component_type,
	                                                const ut::Function<ut::Optional<ut::Error>(ut::meta::Snapshot&)>& callback,
	                                                const ut::String& parameter_name) = 0;

	// Tells the environment to exit after current tick ends.
	virtual void Exit() = 0;
};

// ve::Environment is a class that contains all information about the virtual
// environment (like entities and systems), and processes events and
// interactions between entities.
class Environment : private CmdAccessibleEnvironment, public ut::NonCopyable
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

private:
	// Adds a new entity to the environment.
	//    @param entity - new entity object.
	//    @return - id of the entity or ut::Error if failed to
	//              add @entity to the environment.
	ut::Result<Entity::Id, ut::Error> AddEntity(Entity entity) override;

	// Adds a new component to the desired entity.
	//    @param entity_id - identifier of the entity to add the component.
	//    @param component - the unique pointer to the component to be added.
	//    @return - optional ut::Error if failed.
	ut::Optional<ut::Error> AddComponent(Entity::Id entity_id,
	                                     ut::UniquePtr<Component> component) override;

	// Deletes the entity from the environment.
	//    @param entity_id - identifier of the desired entity.
	//    @return - optional ut::Error if failed.
	ut::Optional<ut::Error> DeleteEntity(Entity::Id entity_id) override;

	// Deletes a component from the desired entity.
	//    @param entity_id - identifier of the desired entity.
	//    @param component_type - handle of the component type.
	//    @return - optional ut::Error if failed.
	ut::Optional<ut::Error> DeleteComponent(Entity::Id entity_id,
	                                        ut::DynamicType::Handle component_type) override;

	// Updates the desired component.
	//    @param entity_id - id of the entity owning the desired component.
	//    @param component_type - type of the component.
	//    @param callback - reference to the callback that accepts a reference
	//                      to the meta-snapshot of the desired parameter.
	//    @param parameter_name - name of the parameter to be updated.
	//    @return - optional ut::Error if failed to update the component.
	ut::Optional<ut::Error> UpdateComponent(Entity::Id entity_id,
	                                        ut::DynamicType::Handle component_type,
	                                        const ut::Function<ut::Optional<ut::Error>(ut::meta::Snapshot&)>& callback,
	                                        const ut::String& parameter_name) override;

	// Tells the environment to exit after current tick ends.
	void Exit() override;

	// Processes provided array of commands.
	//    @param commands - array of commands to be processed by
	//                      environment.
	//    @return - optional ut::Error if at least one command
	//              failed to be executed.
	ut::Optional<ut::Error> ExecuteCommands(CmdArray commands);

	// Array of entities, where every entity has a unique id.
	ut::SparseHashMap<Entity::Id, Entity> entities;

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