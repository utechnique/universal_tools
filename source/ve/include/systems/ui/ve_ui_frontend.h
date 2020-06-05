//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_dedicated_thread.h"
#include "ve_ui_viewport.h"
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
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//