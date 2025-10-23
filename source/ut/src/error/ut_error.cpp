//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "error/ut_error.h"
#include "dbg/ut_backtrace.h"
#include "thread/ut_mutex.h"
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
//    @param backtrace - indicates if a callstack must be built.
Error::Error(error::Code error_code,
             bool backtrace) : code(error_code)
{
#if UT_ERROR_BACKTRACE
	if (backtrace)
	{
		GetCallstack();
	}
#endif
}

//----------------------------------------------------------------------------->
// Constructor
//    @param error_code - error code, see ut::error::Code enumeration
//    @param error_desc - custom (user-defined) error description
//    @param backtrace - indicates if a callstack must be built.
Error::Error(error::Code error_code,
             String error_desc,
             bool backtrace) : code(error_code)
#if UT_ERROR_DESCRIPTION
                                , description(Move(error_desc))
#endif
{
#if UT_ERROR_BACKTRACE
	if (backtrace)
	{
		GetCallstack();
	}
#endif
}

//----------------------------------------------------------------------------->
// Returns error code
//    @return - error code, see ut::error::Code enumeration
error::Code Error::GetCode() const
{
	return code;
}

//----------------------------------------------------------------------------->
// Returns full description of the error
// 	  @param short_form - indicates whether to form a description
//                        in a short single line form without a backtrace.
//    @return - string with error description and call stack
const String Error::GetDesc(bool short_form) const
{
	String desc(error::GetCodeDesc(code));

	// full description
#if UT_ERROR_DESCRIPTION
	if (description.Length() > 0)
	{
		if (short_form)
		{
			desc += String(" ") + description;
		}
		else
		{
			desc.AddLine("");
			desc.AddLine("Detailed description:");
			desc.AddLine(description);
		}
	}
#endif // UT_ERROR_DESCRIPTION

	// callstack
#if UT_ERROR_BACKTRACE
	if (!short_form)
	{
		desc.AddLine("");
		desc.AddLine("Call Stack:");
		desc.AddLine(callstack);
	}
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

	for (size_t i = 0; i < symbols.Count(); i++)
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
