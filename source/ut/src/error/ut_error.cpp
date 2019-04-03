//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "error/ut_error.h"
#include "dbg/ut_backtrace.h"
#include "system/ut_mutex.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Tracing callstack can be simultaneously performed only from one thread.
// Therefore some synchronization primitive is needed to perform it correctly.
// ut::g_backtrace_mutex is global-scope mutex to synchronize backtracing.
Mutex g_backtrace_mutex;

//----------------------------------------------------------------------------//
// Constructor
//    @param error_code - error code, see ut::error::Code enumeration
Error::Error(error::Code error_code) : code(error_code)
{
	GetCallstack();
}

//----------------------------------------------------------------------------->
// Constructor
//    @param error_code - error code, see ut::error::Code enumeration
//    @param error_desc - custom (user-defined) error description
Error::Error(error::Code error_code,
             const String& error_desc) : code(error_code)
#if UT_ERROR_DESCRIPTION
                                       , description(error_desc)
#endif
{
#if UT_ERROR_BACKTRACE
	GetCallstack();
#endif
}

//----------------------------------------------------------------------------->
// Constructor
//    @param error_code - error code, see ut::error::Code enumeration
//    @param error_desc - custom (user-defined) error description
#if CPP_STANDARD >= 2011
Error::Error(error::Code error_code,
             String && error_desc) : code(error_code)
#if UT_ERROR_DESCRIPTION
                                   , description(Move(error_desc))
#endif
{
#if UT_ERROR_BACKTRACE
	GetCallstack();
#endif
}
#endif

//----------------------------------------------------------------------------->
// Copy constructor
Error::Error(const Error& copy) : code(copy.code)
#if UT_ERROR_DESCRIPTION
                                , description(copy.description)
#endif
#if UT_ERROR_BACKTRACE
                                , callstack(copy.callstack)
#endif
{ }

//----------------------------------------------------------------------------->
// Move constructor
#if CPP_STANDARD >= 2011
Error::Error(Error && copy) : code(copy.code)
#if UT_ERROR_DESCRIPTION
                            , description(Move(copy.description))
#endif
#if UT_ERROR_BACKTRACE
                            , callstack(Move(copy.callstack))
#endif
{ }
#endif

//----------------------------------------------------------------------------->
// Returns error code
//    @return - error code, see ut::error::Code enumeration
error::Code Error::GetCode() const
{
	return code;
}

//----------------------------------------------------------------------------->
// Returns full description of the error
//    @return - string with error description and call stack
const String Error::GetDesc() const
{
	//String desc(GetErrorDesc(code));
	String desc(GetErrorDesc(code));

	// full description
#if UT_ERROR_DESCRIPTION
	if (description.Length() > 0)
	{
		desc.AddLine("");
		desc.AddLine("Detailed description:");
		desc.AddLine(description);
	}
#endif // UT_ERROR_DESCRIPTION

	// callstack
#if UT_ERROR_BACKTRACE
	desc.AddLine("");
	desc.AddLine("Call Stack:");
	desc.AddLine(callstack);
#endif // UT_ERROR_BACKTRACE

	return desc;
}

//----------------------------------------------------------------------------->
// Returns a string with a callstack (every call has a separate line)
void Error::GetCallstack()
{
#if UT_ERROR_BACKTRACE
	g_backtrace_mutex.Lock();
	const Array<String> symbols = SymbolsBacktrace();
	g_backtrace_mutex.Unlock();

	for (size_t i = 0; i < symbols.GetNum(); i++)
	{
		if (i == 0)
		{
			callstack = symbols[i];
		}
		else
		{
			callstack.AddLine(symbols[i]);
		}
	}
#endif // UT_ERROR_BACKTRACE
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
