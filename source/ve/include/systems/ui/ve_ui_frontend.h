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
	virtual ~Frontend() = default;

	// When ui exits.
	void ConnectExitSignalSlot(ut::Function<void()> slot);

	// One can start iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	virtual ut::Array< ut::Ref<Viewport> >::Iterator BeginViewports() = 0;

	// One can end iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	virtual ut::Array< ut::Ref<Viewport> >::Iterator EndViewports() = 0;

	// One can start iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	virtual ut::Array< ut::Ref<Viewport> >::ConstIterator BeginViewports() const = 0;

	// One can end iterating viewports by calling this function.
	//    @return - viewport iterator, elements can be modified.
	virtual ut::Array< ut::Ref<Viewport> >::ConstIterator EndViewports() const = 0;

protected:
	// signals
	ut::Signal<void()> exit_signal;

	// title of the application
	static const char* skTitle;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//