//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "dbg_test.h"
//----------------------------------------------------------------------------//
// Unit
DbgTestUnit::DbgTestUnit() : TestUnit("DEBUG")
{
	tasks.Add(ut::MakeUnique<BacktraceTask>());
}

//----------------------------------------------------------------------------//
// Host name
BacktraceTask::BacktraceTask() : TestTask("Backtrace") {}

void BacktraceTask::Execute()
{
	report += "testing errors and backtrace behaviour (YOU WILL SEE ERROR, IT'S OK!):\n";
	try
	{
		ut::Result<ut::String, ut::Error> result = GetStrDbgError(0);
	}
	catch (const ut::Error& error)
	{
		report += error.GetDesc();
	}
}

//----------------------------------------------------------------------------//
// Returns an error if a == 0 to test error handling functionality
ut::Result<ut::String, ut::Error> GetStrDbgError(int a)
{
	if (a == 0)
	{
		ut::ThrowError(ut::error::not_supported);
		return ut::MakeError(ut::error::not_supported);
	}
	else
	{
		return ut::String("mongrel monad test");
	}
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//