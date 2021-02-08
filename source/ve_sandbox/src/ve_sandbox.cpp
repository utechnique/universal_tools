//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ve_sandbox.h"
#include "commands/ve_cmd_add_entity.h"
#include "components/ve_render_component.h"
#include "components/ve_transform_component.h"
#include "components/ve_camera_component.h"
#include "components/ve_free_camera_controller_component.h"
#include "systems/render/units/ve_render_view.h"
//----------------------------------------------------------------------------//
// Creates and runs virtual environment.
void LaunchVirtualEnvironment()
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
		ut::log << exit_error->GetDesc();
	}
	else
	{
		ut::log << "Exited successfully." << ut::cret;
	}

	// finish logging
	ut::log.End();
}

// Entry point.
int UT_MAIN(UT_MAIN_ARG)
{
	ut::CatchExceptions(LaunchVirtualEnvironment);

	// exit
	return 0;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//