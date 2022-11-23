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
//    @param pool - reference to the thread pool to
//                  process parallel children simultaneously.
// 1. Updates own system.
// 2. Updates systems in parallel children pipelines simultaneously in
//    different threads.
// 3. Updates systems in serial children pipelines sequentially one by one.
//    @return - array of commands to be executed by owning environment.
System::Result Pipeline::Execute(ut::ThreadPool<System::Result>& pool)
{
	// array of commands to be returned in the end of the function
	CmdArray commands;

	// update system
	if (system)
	{
		System::Result update_result(system->Update());
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
		scheduler.Enqueue(ut::MakeUnique<PoolTask>(execute, pool));
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
		System::Result execute_result = series[i].Execute(pool);
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
	// register in own system
	if (system)
	{
		system->RegisterEntity(id, entity);
	}

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
}

// Iterates all systems and unregisters a provided entity.
//    @param id - id of the provided entity.
void Pipeline::UnregisterEntity(Entity::Id id)
{
	// unregister in own system
	if (system)
	{
		system->UnregisterEntity(id);
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

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//