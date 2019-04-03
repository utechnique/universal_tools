//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
// base headers
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h> // threads, mutexes, etc
#include <termios.h> // console
#include <errno.h> // error codes

// network headers
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <netdb.h>

// android is unix-like os
#if UT_ANDROID
#	include <jni.h> // native code
#	include <unwind.h> // for backtrace
#else
#	include <execinfo.h> // for backtrace
#	include <iconv.h> // string conversion
#endif

// headers for debug version
#if DEBUG
#	include <dlfcn.h> // for dladdr
#	include <cxxabi.h> // for __cxa_demangle
#endif

// platform-specific macros
#define FORCEINLINE __attribute__((always_inline))

// detect architecture
#if __x86_64__ || __ppc64__
#	define UT_PLATFORM_64BITS 1
#else
#	define UT_PLATFORM_32BITS 1
#endif

// entry point for unix platform
#define UT_MAIN main

// signature of the entry point
#define UT_MAIN_ARG

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//