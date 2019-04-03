//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "error/ut_system_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Converts standard errno code @code into ut::error::Code value
//    @param @code - errno code
//    @return - ut::error::Code equivalent of the @code
error::Code ConvertErrno(int code)
{
	switch(code)
	{
		case EINVAL:       return error::invalid_arg;
		case EBADF:        return error::invalid_arg;
		case EACCES:       return error::access_denied;
		case EPERM:        return error::access_denied;
		case ENOENT:       return error::no_such_file;
		case EBUSY:        return error::busy;
		case ENOTEMPTY:    return error::not_empty;
		case EEXIST:       return error::already_exists;
		case EISDIR:       return error::is_a_directory;
		case ENOTDIR:      return error::not_a_directory;
		case ENAMETOOLONG: return error::name_too_long;
		case EROFS:        return error::access_denied;
		case EXDEV:        return error::not_supported;
#if CPP_STANDARD >= 2011
		case EADDRINUSE:   return error::address_in_use;
#endif
		default:           return error::fail;
	}
}

//----------------------------------------------------------------------------//
// Converts windows system error @code into ut::error::Code value
//    @param @code - windows system error
//    @return - ut::error::Code equivalent of the @code
#if UT_WINDOWS
error::Code ConvertWinSysErr(DWORD code)
{
	switch(code)
	{
		case ERROR_FILE_NOT_FOUND:    return error::no_such_file;
		case ERROR_PATH_NOT_FOUND:    return error::no_such_file;
		case ERROR_ACCESS_DENIED:     return error::access_denied;
		case ERROR_INVALID_ACCESS:    return error::access_denied;
		case ERROR_NOT_ENOUGH_MEMORY: return error::out_of_memory;
		case ERROR_OUTOFMEMORY:       return error::out_of_memory;
		default:                      return error::fail;
	}
}
#endif // UT_WINDOWS

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
