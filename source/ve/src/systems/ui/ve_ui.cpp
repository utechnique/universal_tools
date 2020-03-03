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
void UIDevice::ConnectExitSignalSlot(const ut::Function<void()>& slot)
{
	exit_signal.Connect(slot);
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param ui_device - reference to the ui device to run.
UIJob::UIJob(UIDevice& ui_device) : device(ui_device)
{}

// Calls ve::UIDevice::Run().
void UIJob::Execute()
{
	device.Run();
}

//----------------------------------------------------------------------------//
// Constructor.
UI::UI() : System("ui"), device(new PlatformUIDevice)
{
	// connect ui slots
	ut::MemberInvoker<void(UI::*)()> exit_invoker(&UI::AddExitTask, this);
	device->ConnectExitSignalSlot(exit_invoker);

	// run ui in a separate thread
	ut::UniquePtr<ut::Job> job(new UIJob(device.GetRef()));
	thread = new ut::Thread(ut::Move(job));
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
	UiTaskBuffer& task_array = task_buffer.Lock();
	UiTaskBuffer tasks = ut::Move(task_array);
#if CPP_STANDARD < 2011
	task_array.Empty();
#endif
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
	ut::MemberInvoker<System::Result(UI::*)()> exit_invoker(&UI::Exit, this);
	UiTaskPtr exit_task(new ut::Task<System::Result()>(exit_invoker));

	// add task to the buffer
	ut::ScopeSyncLock< ut::Array<UiTaskPtr> > lock(task_buffer);
	lock.Get().Add(ut::Move(exit_task));
}

// Returns ve::CmdExit command.
System::Result UI::Exit()
{
	CmdArray commands;
	ut::UniquePtr<Cmd> exit_cmd(new CmdExit);
	commands.Add(ut::Move(exit_cmd));
	return commands;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//