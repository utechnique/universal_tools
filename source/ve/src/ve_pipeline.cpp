//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_pipeline.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor, creates empty array of commands.
PipelineCombiner::PipelineCombiner()
{}

// Concatenates provided result to the current array of commands.
//    @param result - return value of the ve::Pipeline::Execute function.
void PipelineCombiner::operator()(System::Result result)
{
	// check if current result already contains an error
	if (!sum)
	{
		return;
	}

	// check if new portion of commands contains an error
	if (!result)
	{
		sum = ut::Move(result);
		return;
	}

	// add commands to the current result
	CmdArray& commands = sum.Get();
	commands += result.Move();
}

// Returns current array of commands, original array becomes empty.
System::Result PipelineCombiner::Move()
{
	ut::ScopeLock lock(mutex);
	return ut::Move(sum);
}

//----------------------------------------------------------------------------//
// Constructor, managed system remains empty.
Pipeline::Pipeline()
{}

// Constructor.
//    @param sys - shared pointer to the managed system.
Pipeline::Pipeline(ut::SharedPtr<System> sys) : system(ut::Move(sys))
{}

// Adds child pipeline that will be executed simultaneously with
// other parallel pipelines during ve::Pipeline::Execute() call.
//    @param pipeline - pipeline object to be added.
//    @return - optional ut::Error if failed to add provided pipeline.
ut::Optional<ut::Error> Pipeline::AddParallel(Pipeline pipeline)
{
	if (!parallel.Add(ut::Move(pipeline)))
	{
		return ut::Error(ut::error::out_of_memory);
	}
	return ut::Optional<ut::Error>();
}

// Adds child pipeline that will be executed in series with
// other serial pipelines during ve::Pipeline::Execute() call.
//    @param pipeline - pipeline object to be added.
//    @return - optional ut::Error if failed to add provided pipeline.
ut::Optional<ut::Error> Pipeline::AddSerial(Pipeline pipeline)
{
	if (!series.Add(ut::Move(pipeline)))
	{
		return ut::Error(ut::error::out_of_memory);
	}
	return ut::Optional<ut::Error>();
}

// Updates systems in the following order:
// 1. Updates own system.
// 2. Updates systems in parallel children pipelines simultaneously in
//    different threads.
// 3. Updates systems in serial children pipelines sequentially one by one.
//    @param time_step_ms - time step for the current frame in milliseconds.
//    @param pool - reference to the thread pool to
//                  process parallel child pipelines simultaneously.
//    @return - array of commands to be executed by owning environment.
System::Result Pipeline::Execute(System::Time time_step_ms,
                                 ut::ThreadPool<System::Result>& pool)
{
	// array of commands to be returned in the end of the function
	CmdArray commands;

	// update system
	if (system)
	{
		System::Result update_result(system->Update(time_step_ms,
		                                            component_access));
		if (!update_result)
		{
			return update_result;
		}
		commands += update_result.Move();
	}

	// kick off parallel tasks
	ut::Scheduler<System::Result, PipelineCombiner> scheduler = pool.CreateScheduler<PipelineCombiner>();
	const size_t parallel_count = parallel.Count();
	for (size_t i = 0; i < parallel_count; i++)
	{
		auto execute = ut::MemberFunction<Pipeline, TaskSignature>(&parallel[i], &Pipeline::Execute);
		scheduler.Enqueue(ut::MakeUnique<PoolTask>(execute, time_step_ms, pool));
	}

	// wait for all tasks to finish and combine all commands in a single array
	PipelineCombiner& combiner = scheduler.WaitForCompletion();
	System::Result parallel_result(combiner.Move());
	if (!parallel_result)
	{
		return parallel_result;
	}
	commands += parallel_result.Move();

	// execute serial tasks
	const size_t serial_count = series.Count();
	for (size_t i = 0; i < serial_count; i++)
	{
		System::Result execute_result = series[i].Execute(time_step_ms, pool);
		if (!execute_result)
		{
			return execute_result;
		}
		commands += execute_result.Move();
	}

	// success
	return commands;
}

// Iterates all systems and registers a provided entity.
//    @param id - id of the provided entity.
//    @param entity - reference to the entity to be registered.
void Pipeline::RegisterEntity(Entity::Id id, Entity& entity)
{
	// register parallel
	const size_t parallel_count = parallel.Count();
	for (size_t i = 0; i < parallel_count; i++)
	{
		parallel[i].RegisterEntity(id, entity);
	}

	// register series
	const size_t serial_count = series.Count();
	for (size_t i = 0; i < serial_count; i++)
	{
		series[i].RegisterEntity(id, entity);
	}

	// check if the set of components of the provided entity
	// fits the managed system requirements
	if (system && !component_access.IsEntityRegistered(id) &&
	    component_access.RegisterEntity(id, entity))
	{
		system->RegisterEntity(id, component_access);
	}
}

// Iterates all systems and unregisters a provided entity.
//    @param id - id of the provided entity.
void Pipeline::UnregisterEntity(Entity::Id id)
{
	// unregister in own system
	if (system && component_access.IsEntityRegistered(id))
	{
		system->UnregisterEntity(id, component_access);
		component_access.UnregisterEntity(id);
	}

	// unregister parallel
	const size_t parallel_count = parallel.Count();
	for (size_t i = 0; i < parallel_count; i++)
	{
		parallel[i].UnregisterEntity(id);
	}

	// unregister series
	const size_t serial_count = series.Count();
	for (size_t i = 0; i < serial_count; i++)
	{
		series[i].UnregisterEntity(id);
	}
}

// Synchronizes the returned component map collection with the internal
// access map so that the both maps shared the same source.
//    @return - a reference to the component map collection to
//              synchronize @component_access with.
ComponentMapCollection<ComponentMap::Access::read_write> Pipeline::SynchronizeComponents()
{
	ComponentMapCollection<ComponentMap::Access::read_write> map_collection;

	// Create empty component maps from systems demands.
	CollectComponentMaps(map_collection);

	// Synchronize component maps with access interface for all systems.
	SynchronizeComponents(map_collection);

	return map_collection;
}

// Collects component maps from all internal systems.
//    @param map_collection -  reference to the component map collection to
//                             store component maps in.
void Pipeline::CollectComponentMaps(ComponentMapCollection<ComponentMap::Access::read_write>& map_collection)
{
	// extract component maps from component sets of the current system 
	if (system)
	{
		ut::Array< ComponentSet<ComponentMap::Access::read_write> > component_sets = system->DefineComponentSets();
		ut::Array< ComponentSet<ComponentMap::Access::read_write> >::Iterator component_set_iterator;
		ComponentMapCollection<ComponentMap::Access::read_write>::Iterator component_map_iterator;
		for (component_set_iterator = component_sets.Begin();
		     component_set_iterator != component_sets.End();
		     ++component_set_iterator)
		{
			for (component_map_iterator = component_set_iterator->component_maps.Begin();
			     component_map_iterator != component_set_iterator->component_maps.End();
			     ++component_map_iterator)
			{
				const ut::DynamicType::Handle component_type = component_map_iterator->GetFirst();
				map_collection.Insert(component_type, ut::Move(component_map_iterator->second));
			}
		}
	}

	// synchronize parallel
	const size_t parallel_count = parallel.Count();
	for (size_t i = 0; i < parallel_count; i++)
	{
		parallel[i].CollectComponentMaps(map_collection);
	}

	// synchronize series
	const size_t serial_count = series.Count();
	for (size_t i = 0; i < serial_count; i++)
	{
		series[i].CollectComponentMaps(map_collection);
	}
}

// Recursively synchronizes the provided component map collection with the
// internal access map so that the both maps shared the same source.
//    @param source - a reference to the component map collection to
//                    synchronize @component_access with.
void Pipeline::SynchronizeComponents(ComponentMapCollection<ComponentMap::Access::read_write>& source)
{
	// synchronize parallel
	const size_t parallel_count = parallel.Count();
	for (size_t i = 0; i < parallel_count; i++)
	{
		parallel[i].SynchronizeComponents(source);
	}

	// synchronize series
	const size_t serial_count = series.Count();
	for (size_t i = 0; i < serial_count; i++)
	{
		series[i].SynchronizeComponents(source);
	}

	// exit if no system
	if (!system)
	{
		return;
	}

	// generate component sets for system access
	ut::Array< ComponentSet<ComponentMap::Access::read_write> > system_sets = system->DefineComponentSets();
	const bool accept_all_entities = system_sets.IsEmpty();
	ut::Array< ComponentSet<ComponentMap::Access::read> > access_sets;
	if (accept_all_entities)
	{
		ComponentSet<ComponentMap::Access::read> access_set;
		access_set.operation = Component::EntityAssociationOperation::unite;

		ComponentMapCollection<ComponentMap::Access::read_write>::Iterator map_iterator;
		for (map_iterator = source.Begin();
		     map_iterator != source.End();
		     ++map_iterator)
		{
			access_set.component_maps.Insert(map_iterator->GetFirst(), map_iterator->GetSecond());
		}

		access_sets.Add(ut::Move(access_set));
	}
	else
	{
		ut::Array< ComponentSet<ComponentMap::Access::read_write> >::Iterator sys_set_iterator;
		for (sys_set_iterator = system_sets.Begin();
		     sys_set_iterator != system_sets.End();
		     ++sys_set_iterator)
		{
			ComponentSet<ComponentMap::Access::read> access_set;
			access_set.operation = sys_set_iterator->operation;
			
			ComponentMapCollection<ComponentMap::Access::read_write>::Iterator map_iterator;
			for (map_iterator = sys_set_iterator->component_maps.Begin();
			     map_iterator != sys_set_iterator->component_maps.End();
			     ++map_iterator)
			{
				const ut::DynamicType::Handle component_type = map_iterator->GetFirst();
				ut::Optional<SharedComponentMap<ComponentMap::Access::read_write>::Type&> component_map = source.Find(component_type);
				access_set.component_maps.Insert(component_type, component_map.Get());
			}

			access_sets.Add(ut::Move(access_set));
		}
	}

	// synchronize component maps between the source and the @component_access
	component_access = SynchronizableComponentAccessGroup(access_sets);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//