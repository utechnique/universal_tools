//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "dbg/ut_exception.h"
#include "dbg/ut_log.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Handles std::exception and ut::Error exceptions thrown by the provided
// function. In case any of these was catched - writes message to log.
//    @param proc - function to be called and handled.
//    @return - 'true' if catched an exception, 'false' otherwise.
bool CatchExceptions(ut::Function<void()> proc)
{
	try
	{
		proc();
	}
	catch (const ut::Error& error)
	{
		ut::log << error.GetDesc() << ut::cret;
		return true;
	}
	catch (const std::exception& exception)
	{
		ut::log << exception.what() << ut::cret;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//