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
// Destructor destroys entities.
Environment::~Environment()
{
	// the rule is that entities must be deleted before systems
	entities.Empty();
}

//----------------------------------------------------------------------------->
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

//----------------------------------------------------------------------------->
// Enqueues a command, it will be executed at
// the beginning of the next tick.
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

//----------------------------------------------------------------------------->
// Adds a new entity to the environment. This function is unsafe if
// this environment is already running, enqueue ve::CmdAddEntity command
// instead.
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