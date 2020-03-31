//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "ve_dedicated_thread.h"
#include "ve_ui_platform.h"
#include "ve_ui_viewport.h"
//----------------------------------------------------------------------------//
#define VE_DESKTOP 1
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::Frontend is an abstract class for ui wrappers. Those wrappers are
// supposed to contain and process actual widgets (that are specific to the
// current platform).
class Frontend : public ut::NonCopyable
{
public:
	// dedicated thread for the ui frontend
	typedef DedicatedThread< ut::UniquePtr<Frontend> > Thread;

	// Destructor.
	virtual ~Frontend() {};

	// When ui exits.
	void ConnectExitSignalSlot(ut::Function<void()> slot); 

	// One can start iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::UniquePtr<Viewport> >::Iterator BeginViewports();

	// One can end iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::UniquePtr<Viewport> >::Iterator EndViewports();

	// One can start iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::UniquePtr<Viewport> >::ConstIterator BeginViewports() const;

	// One can end iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	ut::Array< ut::UniquePtr<Viewport> >::ConstIterator EndViewports() const;

protected:
	// signals
	ut::Signal<void()> exit_signal;

	// title of the application
	static const char* skTitle;

	// viewports are windows showing visual part of environment to user
	ut::Array< ut::UniquePtr<Viewport> > viewports;
};

//----------------------------------------------------------------------------//
// ve::ui::Backend is a system processing ui events.
class Backend : public System
{
	typedef ut::BaseTask<System::Result> UiTask;
	typedef ut::UniquePtr<UiTask> UiTaskPtr;
	typedef ut::Array<UiTaskPtr> UiTaskBuffer;

public:
	// Constructor.
	Backend(ut::SharedPtr<Frontend::Thread> in_frontend_thread);

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update();

private:
	void ConnectFrontendSignals(Frontend& frontend);

	// Adds ve::UI::Exit() function to the task buffer.
	void AddExitTask();

	// Returns ve::CmdExit command.
	System::Result Exit();

	// Thread-safe buffer for ui tasks.
	ut::Synchronized<UiTaskBuffer> task_buffer;

	// UI shell with actual widgets.
	ut::SharedPtr<Frontend::Thread> frontend_thread;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//