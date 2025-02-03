//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
#include "test_manager.h"
#include "file_test.h"
//----------------------------------------------------------------------------//
// Executes test tasks.
void RunTest()
{
	TestManager test_manager;
	test_manager.Execute();
}

// Opens console and log, executes a test, handles input.
void PerformTest()
{
	// allocate console before using
	ut::console.Open();

	// start log
	ut::Optional<ut::Error> log_error = ut::log.Start(g_test_dir + "log.txt", true);
	if (log_error)
	{
		ut::console << "[ERROR] Failed to start log:" << ut::cret << log_error->GetDesc();
	}

	// run test and handle exceptions
	if (ut::CatchExceptions(RunTest))
	{
		ut::log << ut::cret << "[ERROR] Exception catched." << ut::cret;
	}

	// finish
	ut::log << ut::cret << ut::log.timestamp << "[EXIT] Test finished. Press any key to exit." << ut::cret;

	// wait for input before exit
	ut::Result<ut::String, ut::Error> input_result = ut::console.GetLine();

	// show input
	ut::log << ut::cret << "[INPUT] You entered: " << input_result.Get() << ". Waiting a second before exit..";

	// wait before exit
	ut::this_thread::Sleep<ut::time::Unit::second>(1);

	// finish logging
	ut::log.End();

	// close console object
	ut::console.Close();
}

// Entry point.
int main()
{
	ut::CatchExceptions(PerformTest);

	// exit
	return 0;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
