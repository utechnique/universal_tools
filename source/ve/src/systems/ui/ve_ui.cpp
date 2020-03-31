//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui.h"
#include "systems/ui/desktop/ve_desktop_ui.h"
#include "commands/ve_cmd_exit.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
#if VE_DESKTOP
typedef DesktopFrontend PlatformFrontend;
#else
#error PlatformUIDevice is not implemented
#endif

//----------------------------------------------------------------------------//
// Title of the application.
const char* Frontend::skTitle = "Virtual Environment";

// When ui exits.
void Frontend::ConnectExitSignalSlot(ut::Function<void()> slot)
{
	exit_signal.Connect(slot);
}

// One can start iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::Iterator Frontend::BeginViewports()
{
	return viewports.Begin();
}

// One can end iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::Iterator Frontend::EndViewports()
{
	return viewports.End();
}

// One can start iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::ConstIterator Frontend::BeginViewports() const
{
	return viewports.Begin();
}

// One can end iterating viewports by calling this function.
//    @return - viewport iterator, elements can be modified.
ut::Array< ut::UniquePtr<Viewport> >::ConstIterator Frontend::EndViewports() const
{
	return viewports.End();
}

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
		commands += result.MoveResult();
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