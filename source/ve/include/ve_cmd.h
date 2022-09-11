//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Forward declaration.
class CmdAccessibleEnvironment;

//----------------------------------------------------------------------------//
// ve::Cmd is a base class for environment commands. Environment commands are
// generated by systems and executed by environment at the end of every tick.
class Cmd
{
public:
	// Executes the command.
	//    @param environment - reference to the environment
	//                         executing the command.
	//    @return - optional ut::Error if environment failed to execute
	//              the command.
	virtual ut::Optional<ut::Error> Execute(CmdAccessibleEnvironment& environment) = 0;

	// ve::Component is abstract class, therefore must have virtual destructor.
	virtual ~Cmd();
};

// Type of the array of environment commands.
typedef ut::Array< ut::UniquePtr<Cmd> > CmdArray;

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//