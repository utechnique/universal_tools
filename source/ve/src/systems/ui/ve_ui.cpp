//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui.h"
#include "systems/ui/desktop/ve_desktop_ui.h"
#include "commands/ve_cmd_exit.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#if VE_DESKTOP
typedef DesktopUI PlatformUIDevice;
#else
#error PlatformUIDevice is not implemented
#endif

//----------------------------------------------------------------------------//
// Title of the application.
const char* UIDevice::skTitle = "Virtual Environment";

// When ui exits.
void UIDevice::ConnectExitSignalSlot(ut::Function<void()> slot)
{
	exit_signal.Connect(slot);
}

//----------------------------------------------------------------------------//
// Constructor.
UI::UI() : System("ui"), device(ut::MakeShared<PlatformUIDevice>())
{
	// connect ui slots
	device->ConnectExitSignalSlot(ut::MemberFunction<UI, void()>(this, &UI::AddExitTask));

	// run ui in a separate thread
	thread = ut::MakeUnique<ut::Thread>([this] { this->device->Run(); } );
}

// Updates system. This function is called once per tick
// by ve::Environment.
//    @return - array of commands to be executed by owning environment,
//              or ut::Error if system encountered fatal error.
System::Result UI::Update()
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

// Adds ve::UI::Exit() function to the task buffer.
void UI::AddExitTask()
{
	// create exit task
	auto exit_function = ut::MemberFunction<UI, System::Result()>(this, &UI::Exit);
	UiTaskPtr exit_task = ut::MakeUnique< ut::Task<System::Result()> >(exit_function);

	// add task to the buffer
	ut::ScopeSyncLock<UiTaskBuffer> lock(task_buffer);
	lock.Get().Add(ut::Move(exit_task));
}

// Returns ve::CmdExit command.
System::Result UI::Exit()
{
	CmdArray commands;
	commands.Add(ut::MakeUnique<CmdExit>());
	return commands;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//