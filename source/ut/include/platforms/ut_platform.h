//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "preprocessor/ut_def.h"
//----------------------------------------------------------------------------//
// define platforms here
#ifndef UT_WINDOWS
#    define UT_WINDOWS 0
#endif
#ifndef UT_UNIX
#    define UT_UNIX 0
#endif

// platform headers
#if UT_WINDOWS
#include "platforms/ut_windows.h"
#elif UT_UNIX
#include "platforms/ut_unix.h"
#else
#error Unknown platform
#endif

// platform architecture
#ifndef UT_PLATFORM_64BITS
#    define UT_PLATFORM_64BITS 0
#endif

#ifndef UT_PLATFORM_32BITS
#    define UT_PLATFORM_32BITS 0
#endif

// define what platforms do not have console
#define UT_NO_NATIVE_CONSOLE 0

// enumeration of supported platforms
enum UT_PLATFORM
{
	UT_PLATFORM_UNKNOWN,
	UT_PLATFORM_WINDOWS,
	UT_PLATFORM_LINUX,
	UT_PLATFORM_ANDROID
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//