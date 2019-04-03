//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// Define socket types
#if UT_WINDOWS
	#define UT_SOCKET_VER 2
	#define UT_SOCKET_ERROR SOCKET_ERROR
	#define UT_INVALID_SOCKET INVALID_SOCKET
	#define UT_LISTEN_QUEUE SOMAXCONN
	typedef SOCKET socket_t;
#elif UT_UNIX
	#define UT_SOCKET_VER 2
	#define UT_SOCKET_ERROR -1
	#define UT_INVALID_SOCKET -1
	#define UT_LISTEN_QUEUE 50
	typedef int socket_t;
#else
	#error socket types are not defined
#endif

// Receive timeout in seconds
#define UT_SOCKET_RECV_TIMEOUT_SEC 10

//----------------------------------------------------------------------------//
// class ut::SocketSystem is needed to initialize socket libraries on startup.
// Do not create any objects, it's for internal use only. Global variable
// ut::socket_system is used to load libraries in constructor and detach them
// in destructor. Do not use this variable either.
class SocketSystem
{
public:
	// Constructor, windows implementation calls WSAStartup(),
	//              linux overrides SIGPIPE signal
	SocketSystem();

	// Destructor, windows implementation calls WSACleanup(), linux does nothing
	~SocketSystem();
};

//----------------------------------------------------------------------------//
// ut::socket_system global variable is used to initialize socket libraries
// via it's constructor and to detach them in destructor. Don't use it.
extern const SocketSystem socket_system;

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//