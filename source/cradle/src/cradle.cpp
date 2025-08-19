//----------------------------------------------------------------------------//
//-----------------------------|  C R A D L E  |----------------------------- //
//----------------------------------------------------------------------------//
#include "cradle.h"
#include "cradle_pipeline.h"
#include "cradle_env_gen.h"
#include "commands/ve_cmd_add_entity.h"
//----------------------------------------------------------------------------//
// Creates and runs virtual environment.
void LaunchEnvironment()
{
	srand(0);

	// start log
	ut::Optional<ut::Error> log_error = ut::log.Start("log.txt");

	// initialize environment
	ve::Pipeline pipeline = cradle::CreatePipeline();
	ve::Environment environment(ut::Move(pipeline));

	// generate entities
	ut::Array< ut::Array< ut::UniquePtr<ve::Component> > > entities = cradle::GenerateEnvironment();
	for (size_t i = 0; i < entities.Count(); i++)
	{
		environment.EnqueueCommand(ut::MakeUnique<ve::CmdAddEntity>(ut::Move(entities[i])));
	}

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
	ut::CatchExceptions(LaunchEnvironment);

	// exit
	return 0;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//