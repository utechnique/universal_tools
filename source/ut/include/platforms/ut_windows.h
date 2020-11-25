//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
// Visual leak detector for profiling.
#if UT_MEMORY_PROFILING && UT_USE_VLD
#	include <vld.h>
#endif

// base headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#include "Winsock2.h" // sockets
#include <windows.h>
#include <errno.h> // error codes
#include <process.h> // threads, mutexes, etc

// headers for debug version
#if DEBUG
#	pragma warning(push, 0)
#		include "dbghelp.h" // callstack backtrace
#	pragma warning(pop)
#endif

// undefine 'CopyFile' because it conflicts
// with ut::CopyFile function
#undef CopyFile

// platform-specific macros
#define FORCEINLINE __forceinline

// detect architecture
#if _WIN64
#	define UT_PLATFORM_64BITS 1
#else
#	define UT_PLATFORM_32BITS 1
#endif

// entry point for windows platform
#define UT_MAIN WinMain

// signature of the entry point
#define UT_MAIN_ARG _In_ HINSTANCE hInstance,    \
                    _In_ HINSTANCE hPrevInstance,\
                    _In_ LPSTR     lpCmdLine,    \
                    _In_ int       nCmdShow

// disable innocuous warnings
#pragma warning(disable:4503)

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//