//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "error/ut_error_codes.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Converts standard errno code @code into ut::error::Code value
//    @param @code - errno code
//    @return - ut::error::Code equivalent of the @code
error::Code ConvertErrno(int code);

// Converts windows system error @code into ut::error::Code value
//    @param @code - windows system error
//    @return - ut::error::Code equivalent of the @code
#if UT_WINDOWS
error::Code ConvertWinSysErr(DWORD code);
#endif // UT_WINDOWS

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
