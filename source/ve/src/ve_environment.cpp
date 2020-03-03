//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Constructor.
//    @param in_pipeline - ve::Pipeline object.
Environment::Environment(Pipeline in_pipeline) : pipeline(ut::Move(in_pipeline))
                                               , exit(false)
{}

//----------------------------------------------------------------------------->
// Main loop.
//    @return - optional ut::Error if environment stopped
//              working due to internal error.
ut::Optional<ut::Error> Environment::Run()
{
	// main loop
	while (!exit.Read())
	{
		// processing pipeline must happen inside a thread pool, so that
		// we could measure and analyze performance
		ut::Scheduler<System::Result, PipelineCombiner> scheduler = pool.CreateScheduler<PipelineCombiner>();
		ut::MemberInvoker<System::Result(Pipeline::*)(ut::ThreadPool<System::Result>&)> invoker(&Pipeline::Execute, &pipeline);
		ut::UniquePtr< ut::BaseTask<System::Result> > task(new ut::Task<System::Result(ut::ThreadPool<System::Result>&)>(invoker, pool));
		scheduler.Enqueue(ut::Move(task));

		// get the result of execution
		PipelineCombiner& combiner = scheduler.WaitForCompletion();
		System::Result execute_result(combiner.MoveResult());
		if (!execute_result)
		{
			return execute_result.MoveAlt();
		}

		// execute environment commands
		ut::Optional<ut::Error> dispatch_error = ExecuteCommands(execute_result.MoveResult());
		if (dispatch_error)
		{
			return dispatch_error;
		}
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Adds a new entity to the environment.
//    @param entity - new entity object.
//    @return - id of the entity or ut::Error if failed to
//              add @entity to the environment.
ut::Result<Entity::Id, ut::Error> Environment::AddEntity(Entity entity)
{
	// generate unique identifier for the new entity
	Entity::Id id = id_generator.Generate();

	// register in pipeline systems
	pipeline.RegisterEntity(id, entity);

	// add entity to the array
	if (!entities.Insert(id, ut::Move(entity)))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// return unique identifier of the new entity
	return id;
}

//----------------------------------------------------------------------------->
// Tells the environment to exit after current tick ends.
void Environment::Exit()
{
	exit.Store(true);
}

//----------------------------------------------------------------------------->
// Processes provided array of commands.
//    @param commands - array of commands to be processed by
//                      environment.
//    @return - optional ut::Error if at least one command
//              failed to be executed.
ut::Optional<ut::Error> Environment::ExecuteCommands(CmdArray commands)
{
	const size_t cmd_count = commands.GetNum();
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