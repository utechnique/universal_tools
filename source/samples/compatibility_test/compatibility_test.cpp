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
		ut::console << "[ERROR] Failed to start log:\n" << log_error.Get().GetDesc();
	}

	// run test and handle exceptions
	if (ut::CatchExceptions(RunTest))
	{
		ut::log << "\n[ERROR] Exception catched.\n";
	}

	// finish
	ut::log << "\n[EXIT] Test finished. Press any key to exit.\n";

	// wait for input before exit
	ut::Result<ut::String, ut::Error> input_result = ut::console.GetLine();

	// show input
	ut::log << "\n[INPUT] You entered: " << input_result.GetResult() << ". Waiting a second before exit..";

	// wait before exit
	ut::this_thread::Sleep(1500);

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
