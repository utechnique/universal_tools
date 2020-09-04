//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_backend.h"
#include "commands/ve_cmd_exit.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor.
Backend::Backend(ut::SharedPtr<Frontend::Thread> in_frontend_thread) : System("ui")
                                                                     , frontend_thread(ut::Move(in_frontend_thread))
{
	// connect ui slots
	auto connect_signals_proc = ut::MemberFunction<Backend, void(Frontend&)>(this, &Backend::ConnectFrontendSignals);
	frontend_thread->Enqueue(ut::Move(connect_signals_proc));
}

// Updates system. This function is called once per tick
// by ve::Environment.
//    @return - array of commands to be executed by owning environment,
//              or ut::Error if system encountered fatal error.
System::Result Backend::Update()
{
	// return value
	ut::Array< ut::UniquePtr<Cmd> > commands;

	// get tasks from the buffer
	UiTaskBuffer tasks = ut::Move(task_buffer.Lock());
	task_buffer.Unlock();

	// execute all tasks and assemble the result
	const size_t task_count = tasks.GetNum();
	for (size_t i = 0; i < task_count; i++)
	{
		System::Result result = tasks[i]->Execute();
		if (!result)
		{
			return result;
		}
		commands += result.Move();
	}

	// return array of commands that are to be
	// executed by owning environment
	return commands;
}

void Backend::ConnectFrontendSignals(Frontend& frontend)
{
	frontend.ConnectExitSignalSlot(ut::MemberFunction<Backend, void()>(this, &Backend::AddExitTask));
}

// Adds ve::Backend::Exit() function to the task buffer.
void Backend::AddExitTask()
{
	// create exit task
	auto exit_function = ut::MemberFunction<Backend, System::Result()>(this, &Backend::Exit);
	UiTaskPtr exit_task = ut::MakeUnique< ut::Task<System::Result()> >(exit_function);

	// add task to the buffer
	ut::ScopeSyncLock<UiTaskBuffer> lock(task_buffer);
	lock.Get().Add(ut::Move(exit_task));
}

// Returns ve::CmdExit command.
System::Result Backend::Exit()
{
	CmdArray commands;
	commands.Add(ut::MakeUnique<CmdExit>());
	return commands;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//