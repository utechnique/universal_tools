//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "error/ut_error_codes.h"
#include "error/ut_error_desc.h"
#include "error/ut_system_error.h"
//----------------------------------------------------------------------------//
#define UT_ERROR_BACKTRACE UT_DEBUG
#define UT_ERROR_DESCRIPTION UT_DEBUG
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Error class contains error code and call stack of the function where
// the error occurred. You must provide error code to construct Error object.
// Use ut::Error::GetCode() to get error code, and ut::Error::GetDesc() to get
// full description of the error with call stack.
class Error
{
public:
	// Constructor
	//    @param error_code - error code, see ut::error::Code enumeration
	//    @param backtrace - indicates if a callstack must be built.
	Error(error::Code error_code, bool backtrace = false);

	// Constructor
	//    @param error_code - error code, see ut::error::Code enumeration
	//    @param error_desc - custom (user-defined) error description
	//    @param backtrace - indicates if a callstack must be built.
	Error(error::Code error_code, String error_desc, bool backtrace = false);

	// Constructor
	//    @param error_code - error code, see ut::error::Code enumeration
	//    @param error_desc - null-terminated C-string with custom
	//                        (user-defined) error description
	//    @param backtrace - indicates if a callstack must be built.
	Error(error::Code error_code,
	      const char* error_desc,
	      bool backtrace = false) : Error(error_code,
	                                      String(error_desc),
	                                      backtrace)
	{}

	// Returns error code
	//    @return - error code, see ut::error::Code enumeration
	error::Code GetCode() const;

	// Returns full description of the error
	// 	  @param short_form - indicates whether to form a description
	//                        in a short single line form without a backtrace.
	//    @return - string with error description and call stack
	const String GetDesc(bool short_form = false) const;

private:
	// Returns a string with a callstack (every call has a separate line)
	void GetCallstack();

	// error code
	error::Code code;

#if UT_ERROR_DESCRIPTION
	// full description of the error (can be empty)
	String description;
#endif

#if UT_ERROR_BACKTRACE
	// call stack of the function where the error occurred
	String callstack;
#endif
};

//----------------------------------------------------------------------------//
// Shorter forms of the ut::MakeAlt<ut::Error>()
inline Alternate<Error> MakeError(const Error& err)
{
	return MakeAlt<ut::Error>(err);
}

inline Alternate<Error> MakeError(Error&& err)
{
	return MakeAlt<ut::Error>(ut::Move(err));
}

inline Alternate<Error> MakeError(error::Code code, String desc)
{
	return MakeError(ut::Error(code, ut::Move(desc)));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//