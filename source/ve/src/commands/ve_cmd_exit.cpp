//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commands/ve_cmd_exit.h"
#include "ve_environment.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Calls ve::Environment::Exit().
//    @param environment - reference to the environment
//                         executing the command.
//    @return - optional ut::Error if environment failed to execute
//              the command.
ut::Optional<ut::Error> CmdExit::Execute(CmdAccessibleEnvironment& environment)
{
	// send exit request
	environment.Exit();

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//