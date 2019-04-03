//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
#include "test_manager.h"
#include "file_test.h"
//----------------------------------------------------------------------------//
// Entry point.
int main()
{
	try // handle any ut::Error exception
	{
		// allocate console before using
		ut::console.Open();

		// start log
		ut::Optional<ut::Error> log_error = ut::log.Start(g_test_dir + "log.txt", true);
		if (log_error)
		{
			ut::console << "[ERROR] Failed to start log:\n" << log_error.Get().GetDesc();
		}

		// create and execute test manager
		TestManager test_manager;
		test_manager.Execute();

		// finish
		ut::log << "\n[EXIT] Test successfully finished. Press any key to exit.\n";
	}
	catch(const ut::Error& error)
	{
		ut::log << "\n[ERROR] Exception handled:\n" << error.GetDesc() << "\nPress any key to exit.\n";
	}

	// wait for input before exit
	ut::Result<ut::String, ut::Error> input_result = ut::console.GetLine();

	// show input
	ut::log << "\n[INPUT] You entered: " << input_result.GetResult() << ". Waiting a second before exit..";

	// wait before exit
	ut::Sleep(1500);

	// finish logging
	ut::log.End();

	// close console object
	ut::console.Close();

	// exit
	return 0;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//