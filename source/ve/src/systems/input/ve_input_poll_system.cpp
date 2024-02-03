//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/input/ve_input_poll_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(input)
//----------------------------------------------------------------------------//
// Constructor.
PollSystem::PollSystem(ut::SharedPtr<input::Manager> input_mgr_ptr) : System("input_polling")
                                                                    , input_mgr(ut::Move(input_mgr_ptr))
{}

// Updates system. This function is called once per tick.
//    @param time_step_ms - time step for the current frame in milliseconds.
//    @param access - reference to the object providing access to the
//                    desired components.
//    @return - array of commands to be executed by owning environment,
//              or ut::Error if system encountered fatal error.
System::Result PollSystem::Update(System::Time time_step_ms,
                                  ComponentAccessGroup& access)
{
	UT_ASSERT(input_mgr);
	input_mgr->Update();
	return System::Result();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(input)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//