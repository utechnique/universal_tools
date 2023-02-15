//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "ve_ui_frontend.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::Backend is a system processing ui events.
class Backend : public System
{
public:
	// Constructor.
	Backend(ut::SharedPtr<Frontend::Thread> in_frontend_thread);

	// Updates system. This function is called once per tick
	// by ve::Environment.
	//    @param access - reference to the object providing access to the
	//                    desired components.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update(ComponentAccess& access) override;

private:
	// UI shell with actual widgets.
	ut::SharedPtr<Frontend::Thread> frontend_thread;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//