//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "ve_input_manager.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// ve::input::PollSystem polls all input devices and caches the result.
class PollSystem : public System
{
public:
	// Constructor.
	PollSystem(ut::SharedPtr<input::Manager> input_mgr_ptr);

	// Updates system. This function is called once per tick.
	//    @return - array of commands to be executed by owning environment,
	//              or ut::Error if system encountered fatal error.
	System::Result Update();

private:
	ut::SharedPtr<input::Manager> input_mgr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//