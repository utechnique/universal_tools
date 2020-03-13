//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
//----------------------------------------------------------------------------//
#define VE_DESKTOP 1
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::UIDevice is an abstract class for ui wrappers. Those wrappers are
// supposed to contain and process actual widgets (that are specific to the
// current platform).
class UIDevice : public ut::NonCopyable
{
public:
	// Destructor.
	virtual ~UIDevice() {};

	// Runs ui routine.
	virtual void Run() = 0;

	// When ui exits.
	void ConnectExitSignalSlot(ut::Function<void()> slot); 

protected:
	// signals
	ut::Signal<void()> exit_signal;

	// title of the application
	static const char* skTitle;
};

//----------------------------------------------------------------------------//
// ve::UI is a system processing ui events.
class UI : public System
{
	typedef ut::BaseTask<System::Result> UiTask;
	typedef ut::UniquePtr<UiTask> UiTaskPtr;
	typedef ut::Array<UiTaskPtr> UiTaskBuffer;

public:
	// Constructor.
	UI();

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update();

private:
	// Adds ve::UI::Exit() function to the task buffer.
	void AddExitTask();

	// Returns ve::CmdExit command.
	System::Result Exit();

	// UI device with actual widgets.
	ut::SharedPtr<UIDevice> device;

	// Thread for the ui device.
	ut::UniquePtr<ut::Thread> thread;

	// Thread-safe buffer for ui tasks.
	ut::Synchronized<UiTaskBuffer> task_buffer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//