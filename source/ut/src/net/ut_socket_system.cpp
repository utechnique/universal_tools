//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_socket_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// ut::socket_system global variable is used to initialize socket libraries
// via it's constructor and to detach them in destructor. Don't use it.
const SocketSystem socket_system;

//----------------------------------------------------------------------------//
// Constructor, windows implementation calls WSAStartup(),
//              linux overrides SIGPIPE signal
SocketSystem::SocketSystem()
{
#if UT_WINDOWS
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(UT_SOCKET_VER, 0), &wsa_data) == 0)
	{
		if (LOBYTE(wsa_data.wVersion < UT_SOCKET_VER))
		{
			throw Error(error::fail);
		}
	}
	else
	{
		throw Error(error::fail);
	}
#elif UT_UNIX
	signal(SIGPIPE, SIG_IGN);
#else
#error ut::SocketSystem::SocketSystem() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Destructor, windows implementation calls WSACleanup(), linux does nothing
SocketSystem::~SocketSystem()
{
#if UT_WINDOWS
	WSACleanup();
#endif
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//