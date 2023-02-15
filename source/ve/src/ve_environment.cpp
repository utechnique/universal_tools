//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
//    @param in_pipeline - ve::Pipeline object.
CmdAccessibleEnvironment::CmdAccessibleEnvironment(Pipeline in_pipeline) : pipeline(ut::Move(in_pipeline))
                                                                         , exit(false)
                                                                         , components(pipeline.SynchronizeComponents())
{}

// Destructor destroys entities.
CmdAccessibleEnvironment::~CmdAccessibleEnvironment()
{
	// the rule is that entities must be deleted before systems
	entities.Reset();
}

//----------------------------------------------------------------------------->
// Adds a new entity to the environment.
//    @param entity - new entity object.
//    @return - id of the entity or ut::Error if failed to
//              add @entity to the environment.
ut::Result<Entity::Id, ut::Error> CmdAccessibleEnvironment::AddEntity()
{
	// generate unique identifier for the new entity
	Entity::Id id = id_generator.Generate();

	// add entity to the array
	if (entities.Insert(id, Entity()))
	{
		return ut::MakeError(ut::error::already_exists);
	}

	// register in pipeline systems
	ut::Optional<Entity&> entity = entities.Find(id);
	UT_ASSERT(entity.HasValue());
	pipeline.RegisterEntity(id, entity.Get());

	// return unique identifier of the new entity
	return id;
}

// Adds a new component to the desired entity.
//    @param entity_id - identifier of the entity to add the component to.
//    @param component - unique pointer to the component to be added.
//    @return - optional ut::Error if failed.
ut::Optional<ut::Error> CmdAccessibleEnvironment::AddComponent(Entity::Id entity_id,
                                                               ut::UniquePtr<Component> component)
{
	// find desired entity by it's identifier
	ut::Optional<Entity&> entity = entities.Find(entity_id);
	if (!entity)
	{
		return ut::Error(ut::error::not_found);
	}

	// find appropriate component hashmap
	const ut::DynamicType::Handle component_type = component->Identify().GetHandle();
	ut::Optional<SharedComponentMap<ut::access_full>::Type&> map_find_result = components.Find(component_type);
	if (!map_find_result)
	{
		return ut::Error(ut::error::not_supported,
		                 "The component of the desired type is not supported.");
	}

	// check if this component already exists
	if (entity->HasComponent(component_type))
	{
		return ut::Error(ut::error::already_exists);
	}

	// add component
	SharedComponentMap<ut::access_full>::Type& map = map_find_result.Get();
	UT_ASSERT(map.Get() != nullptr);
	const ut::Optional<Component&> existing_component = map->Insert(entity_id, ut::Move(component));
	if (existing_component)
	{
		return ut::Error(ut::error::already_exists);
	}
	entity->AddComponent(component_type);

	// re-register the entity
	pipeline.UnregisterEntity(entity_id);
	pipeline.RegisterEntity(entity_id, entity.Get());

	// success
	return ut::Optional<ut::Error>();
}

// Deletes the entity from the environment.
//    @param entity_id - identifier of the desired entity.
//    @return - optional ut::Error if failed.
ut::Optional<ut::Error> CmdAccessibleEnvironment::DeleteEntity(Entity::Id entity_id)
{
	// find the desired entity
	ut::Optional<Entity&> find_result = entities.Find(entity_id);
	if (!find_result)
	{
		return ut::Error(ut::error::not_found);
	}

	// unregister components
	pipeline.UnregisterEntity(entity_id);

	// remove all components
	const Entity& entity = find_result.Get();
	const size_t component_count = entity.CountComponents();
	for (size_t i = 0; i < component_count; i++)
	{
		const ut::DynamicType::Handle component_type = entity.GetComponentByIndex(i);
		ut::Optional<SharedComponentMap<ut::access_full>::Type&> component_map = components.Find(component_type);
		UT_ASSERT(component_map.HasValue());
		const bool remove_component_result = component_map.Get()->Remove(entity_id);
		UT_ASSERT(remove_component_result);
	}

	// remove entity
	const bool remove_entity_result = entities.Remove(entity_id);
	UT_ASSERT(remove_entity_result);

	// release the identifier so that it could be used by a new entity
	id_generator.Release(entity_id);

	// success
	return ut::Optional<ut::Error>();
}

// Deletes a component from the desired entity.
//    @param entity_id - identifier of the desired entity.
//    @param component_type - handle of the component type.
//    @return - optional ut::Error if failed.
ut::Optional<ut::Error> CmdAccessibleEnvironment::DeleteComponent(Entity::Id entity_id,
                                                                  ut::DynamicType::Handle component_type)
{
	// find the desired entity
	ut::Optional<Entity&> find_result = entities.Find(entity_id);
	if (!find_result)
	{
		return ut::Error(ut::error::not_found);
	}

	// find and remove the desired component
	Entity& entity = find_result.Get();
	const size_t component_count = entity.CountComponents();
	for (size_t i = 0; i < component_count; i++)
	{
		const ut::DynamicType::Handle component_type_handle = entity.GetComponentByIndex(i);
		if (component_type_handle != component_type)
		{
			continue;
		}

		// remove the component
		ut::Optional<SharedComponentMap<ut::access_full>::Type&> component_map = components.Find(component_type);
		UT_ASSERT(component_map.HasValue());
		const bool remove_component_result = component_map.Get()->Remove(entity_id);
		UT_ASSERT(remove_component_result);
		entity.RemoveComponent(i);

		// re-register the entity
		pipeline.UnregisterEntity(entity_id);
		pipeline.RegisterEntity(entity_id, entity);

		// exit
		return ut::Optional<ut::Error>();
	}

	// the component was not found
	return ut::Error(ut::error::not_found);
}

// Updates the desired component.
//    @param entity_id - id of the entity owning the desired component.
//    @param component_type - type of the component.
//    @param callback - reference to the callback that accepts a reference
//                      to the meta-snapshot of the desired parameter.
//    @param parameter_name - name of the parameter to be updated.
//    @return - optional ut::Error if failed to update the component.
ut::Optional<ut::Error> CmdAccessibleEnvironment::UpdateComponent(Entity::Id entity_id,
                                                                  ut::DynamicType::Handle component_type,
                                                                  const ut::Function<ut::Optional<ut::Error>(ut::meta::Snapshot&)>& callback,
                                                                  const ut::String& parameter_name)
{
	UT_ASSERT(callback.IsValid());
	
	// find the component map
	ut::Optional<SharedComponentMap<ut::access_full>::Type&> component_map = components.Find(component_type);
	if (!component_map)
	{
		return ut::Error(ut::error::not_found, "Desired component map was not found.");
	}
	
	// find the desired component by the id of the owning entity
	ut::Optional<Component&> component = component_map.Get()->Find(entity_id);
	if (!component)
	{
		return ut::Error(ut::error::not_found, "Desired component was not found.");
	}

	// capture meta snapshot of the component
	ut::meta::Snapshot component_snapshot = ut::meta::Snapshot::Capture(component.Get());
	ut::Optional<ut::meta::Snapshot&> parameter_snapshot = component_snapshot.FindChildByName(parameter_name);
	if (!parameter_snapshot)
	{
		return ut::Error(ut::error::not_found, "Desired parameter was not found.");
	}

	// success
	return callback(parameter_snapshot.Get());
}

//----------------------------------------------------------------------------->
// Tells the environment to exit after the current tick ends.
void CmdAccessibleEnvironment::Exit()
{
	exit.Store(true);
}

//----------------------------------------------------------------------------->
// Constructor.
//    @param in_pipeline - ve::Pipeline object.
Environment::Environment(Pipeline in_pipeline) : CmdAccessibleEnvironment(ut::Move(in_pipeline))
{}

// Main loop.
//    @return - optional ut::Error if environment stopped
//              working due to internal error.
ut::Optional<ut::Error> Environment::Run()
{
	// main loop
	while (!exit.Read())
	{
		// move pending commands to temporary buffer
		CmdArray& locked_commands = commands.Lock();
		CmdArray cmd_buffer(ut::Move(locked_commands));
		commands.Unlock();

		// execute pending commands
		ut::Optional<ut::Error> cmd_error = ExecuteCommands(ut::Move(cmd_buffer));
		if (cmd_error)
		{
			return cmd_error;
		}

		// processing pipeline must happen inside a thread pool, so that
		// we could measure and analyze performance
		ut::Scheduler<System::Result, PipelineCombiner> scheduler = pool.CreateScheduler<PipelineCombiner>();
		auto execute = ut::MemberFunction<Pipeline, Pipeline::TaskSignature>(&pipeline, &Pipeline::Execute);
		scheduler.Enqueue(ut::MakeUnique<Pipeline::PoolTask>(execute, pool));

		// get the result of execution
		PipelineCombiner& combiner = scheduler.WaitForCompletion();
		System::Result execute_result(combiner.Move());
		if (!execute_result)
		{
			return execute_result.MoveAlt();
		}

		// execute accumulated pipeline commands
		cmd_error = ExecuteCommands(execute_result.Move());
		if (cmd_error)
		{
			return cmd_error;
		}
	}

	// success
	return ut::Optional<ut::Error>();
}

// Enqueues a command, it will be executed at the beginning of the next tick.
//    @param command - unique pointer to the command to be executed.
//    @return - optional ut::Error if failed to add provided command.
ut::Optional<ut::Error> Environment::EnqueueCommand(ut::UniquePtr<Cmd> command)
{
	ut::ScopeSyncLock<CmdArray> cmd_lock(commands);
	if (!cmd_lock.Get().Add(ut::Move(command)))
	{
		return ut::Error(ut::error::out_of_memory);
	}
	return ut::Optional<ut::Error>();
}

// Processes provided array of commands.
//    @param commands - array of commands to be processed by
//                      environment.
//    @return - optional ut::Error if at least one command
//              failed to be executed.
ut::Optional<ut::Error> Environment::ExecuteCommands(CmdArray commands)
{
	const size_t cmd_count = commands.Count();
	for (size_t i = 0; i < cmd_count; i++)
	{
		ut::Optional<ut::Error> cmd_error = commands[i]->Execute(*this);
		if (cmd_error)
		{
			return cmd_error;
		}
	}

	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//