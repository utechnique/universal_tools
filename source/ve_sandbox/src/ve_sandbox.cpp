//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_sandbox.h"
//----------------------------------------------------------------------------//
// Entry point.
int UT_MAIN(UT_MAIN_ARG)
{
	try // handle ut::Error exception
	{
		// start log
		ut::Optional<ut::Error> log_error = ut::log.Start("log.txt");

		// create default environment
		ve::Environment environment;

		// run environment
		ut::Optional<ut::Error> exit_error = environment.Run();

		// process exit code
		if (exit_error)
		{
			ut::log << "Exited with error:" << ut::cret;
			ut::log << exit_error.Get().GetDesc();
		}
		else
		{
			ut::log << "Exited successfully." << ut::cret;
		}
	}
	catch (const ut::Error& error)
	{
		ut::log << error.GetDesc() << ut::cret;
	}

	// finish logging
	ut::log.End();

	// exit
	return 0;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//