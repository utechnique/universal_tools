//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_socket.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
const SocketSystem& Socket::skSystem = socket_system;
//----------------------------------------------------------------------------//
// Default constructor, calls ut::Socket::Create() internally
Socket::Socket()
{
	// create platform socket
	Optional<Error> creation_error = Create();

	// check errors
	if (creation_error)
	{
		throw creation_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Constructor
//    @param ip - can be ipv4/ipv6 address or damain name
//    @param port - port, must be > 0
Socket::Socket(const String& ip, int port)
{
	// create platform socket
	Optional<Error> creation_error = Create();

	// check errors
	if (creation_error)
	{
		throw creation_error.Move();
	}

	// set address
	SetAddr(ip, port);
}

//----------------------------------------------------------------------------->
// Private constructor, for internal use only.
// Arguments are native platform socket and socket address.
Socket::Socket(socket_t      in_socket,
	           sockaddr_in* in_sockaddr) : socket(in_socket)
                                         , sock_addr_in(*in_sockaddr)
{

}

//----------------------------------------------------------------------------->
// Move constructor, moves socket and address
#if CPP_STANDARD >= 2011
Socket::Socket(Socket && copy) : socket(copy.socket)
                               , sock_addr_in(copy.sock_addr_in)
{
	copy.socket = UT_INVALID_SOCKET;
}
#endif

//----------------------------------------------------------------------------->
// Move operator, moves socket and address
#if CPP_STANDARD >= 2011
Socket& Socket::operator = (Socket && copy)
{
	socket = copy.socket;
	sock_addr_in = copy.sock_addr_in;
	copy.socket = UT_INVALID_SOCKET;
	return *this;
}
#endif

//----------------------------------------------------------------------------->
// Destructor, closes @socket, and optionally calls
// ut::Socket::ShutDown() (Linux)
Socket::~Socket(void)
{
#if UT_WINDOWS
	if (socket != UT_INVALID_SOCKET)
	{
		closesocket(socket);
	}
#elif UT_UNIX
	if (socket != UT_INVALID_SOCKET)
	{
		ShutDown();
		close(socket);
	}
#else
	#error ut::Socket::~Socket() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Sets socket address
//    @param ip - can be ipv4/ipv6 address or damain name
//    @param port - port, must be > 0
void Socket::SetAddr(const String& ip, int port)
{
	bool empty = ip.Length() == 0;
	const char* ip_cs = ip.GetAddress();
#if UT_WINDOWS
	sock_addr_in.sin_family = AF_INET;  // TCP/UDP family
	sock_addr_in.sin_port = htons(port);
	sock_addr_in.sin_addr.S_un.S_addr = empty ? INADDR_ANY : inet_addr(ip_cs);
#elif UT_UNIX
	bzero((char*)&sock_addr_in, sizeof(sock_addr_in));
	sock_addr_in.sin_family = AF_INET;  // TCP/UDP family
	sock_addr_in.sin_port = htons(port);
	sock_addr_in.sin_addr.s_addr = empty ? INADDR_ANY : inet_addr(ip_cs);
#else
	#error ut::Socket::SetAddr() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Sets socket address
//    @param address - ut::HostAddress object
void Socket::SetAddr(const HostAddress& address)
{
	SetAddr(address.ip, address.port);
}

//----------------------------------------------------------------------------->
// Returns socket address, or ut::Error if error occurred
Result<HostAddress, Error> Socket::GetAddr() const
{
	// get ip
	const char* out_ip = inet_ntoa(sock_addr_in.sin_addr);
	if (out_ip == nullptr)
	{
		return MakeError(error::fail);
	}

	// get port
	int out_port = ntohs(sock_addr_in.sin_port);

	// return full address
	return HostAddress(String(out_ip), out_port);
}

//----------------------------------------------------------------------------->
// Returns local socket address (for clients) or ut::Error
// if error occurred
Result<HostAddress, Error> Socket::GetLocalAddr() const
{
	struct sockaddr_in local_address;
#if UT_WINDOWS
	int addr_size = sizeof(local_address);
#elif UT_UNIX
	socklen_t addr_size = sizeof(local_address);
#else
	#error ut::Socket::GetLocalAddr() is not implemented
#endif
	if (getsockname(socket, (struct sockaddr*)&local_address, &addr_size) == 0 &&
	   local_address.sin_family == AF_INET && addr_size == sizeof(local_address))
	{
		// get ip
		const char* out_ip = inet_ntoa(local_address.sin_addr);
		if (out_ip == nullptr)
		{
			return MakeError(error::fail);
		}

		// get port
		int out_port = ntohs(local_address.sin_port);

		// return full address
		return HostAddress(String(out_ip), out_port);
	}
	else
	{
#if UT_WINDOWS
		error::Code error_code = error::fail;
#elif UT_UNIX
		error::Code error_code = ConvertErrno(errno);
#endif
		return MakeError(error_code);
	}
}

//----------------------------------------------------------------------------->
// Binds socket
//    @return - ut::Error if error occurred
Optional<Error> Socket::Bind() const
{
#if UT_WINDOWS
	if (bind(socket, (sockaddr*)(&sock_addr_in), sizeof(sock_addr_in)) != 0)
	{
		return error::fail;
	}
#elif UT_UNIX
	// should set SO_REUSEADDR option to prevent timeout after closing a socket
	int opt_val = 1;
	setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(int));

	// now can bind socket
	if (bind(socket, (sockaddr*)(&sock_addr_in), sizeof(sock_addr_in)) != 0)
	{
		return Error(ConvertErrno(errno));
	}
#else
	#error ut::Socket::Bind() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Listens sockets
//    @return - ut::Error if error occurred
Optional<Error> Socket::Listen() const
{
#if UT_WINDOWS
	if (listen(socket, UT_LISTEN_QUEUE) != 0)
	{
		return error::fail;
	}
#elif UT_UNIX
	if (listen(socket, UT_LISTEN_QUEUE) != 0)
	{
		return Error(ConvertErrno(errno));
	}
#else
	#error ut::Socket::Listen() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Accepts socket
//    @return - connected socket pointer if success, or error otherwise
Result<UniquePtr<Socket>, Error> Socket::Accept() const
{
#if UT_WINDOWS
	sockaddr_in client_addr_in;
	int client_addr_size = sizeof(client_addr_in);
	socket_t client_socket = accept(socket, (sockaddr*)(&client_addr_in), &client_addr_size);
	if (client_socket == INVALID_SOCKET)
	{
		return MakeError(error::fail);
	}
	SocketPtr sock_ptr(new Socket(client_socket, &client_addr_in));
	return Move(sock_ptr);
#elif UT_UNIX
	sockaddr_in client_addr_in;
	socklen_t client_addr_size = sizeof(client_addr_in);
	socket_t client_socket = accept(socket, (sockaddr*)(&client_addr_in), &client_addr_size);
	if (client_socket < 0)
	{
		return MakeError(Error(ConvertErrno(errno)));
	}
	SocketPtr sock_ptr(new Socket(client_socket, &client_addr_in));
	return Move(sock_ptr);
#else
	#error ut::Socket::Accept() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Poll socket events
//    @param timeout_ms - timieout in milliseconds
//    @return - ut::Error if error occurred
Optional<Error> Socket::Poll(uint32 timeout_ms) const
{
#if UT_WINDOWS
	WSAPOLLFD fds;
	fds.fd = socket;
	fds.events = POLLRDBAND | POLLRDNORM;
	if (WSAPoll(&fds, 1, timeout_ms) == 0)
	{
		return error::fail;
	}
#elif UT_UNIX
	pollfd fds;
	fds.fd = socket;
	fds.events = POLLIN | POLLPRI;
	if (poll(&fds, 1, timeout_ms) == 0)
	{
		return Error(ConvertErrno(errno));
	}
#else
	#error ut::Socket::Poll() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Shutdown
//    @return - ut::Error if error occurred
Optional<Error> Socket::ShutDown() const
{
#if UT_WINDOWS
	if (shutdown(socket, SD_SEND))
	{
		return error::fail;
	}
#elif UT_UNIX
	if (shutdown(socket, SHUT_RDWR))
	{
		return Error(ConvertErrno(errno));
	}
#else
	#error ut::Socket::ShutDown() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Connects to the listening socket
//    @return - ut::Error if error occurred
Optional<Error> Socket::Connect() const
{
#if UT_WINDOWS
	if (connect(socket, (sockaddr*)(&sock_addr_in), sizeof(sock_addr_in)) != 0)
	{
		return error::fail;
	}
#elif UT_UNIX
	if (connect(socket, (sockaddr*)(&sock_addr_in), sizeof(sock_addr_in)) != 0)
	{
		return Error(ConvertErrno(errno));
	}
#else
	#error ut::Socket::Connect() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Receives data form sender
//    @param sender - use corresponding client socket for servers, or self for clients
//    @param data - pointer to the first byte of data to be sent
//    @param data_size - size of @data in bytes
//    @return - number of received bytes
Result<int, Error> Socket::Recv(byte* data,
                                size_t data_size) const
{
#if UT_WINDOWS
	int bytes = recv(GetPlatformSocket(), (char*)data, (int)data_size, 0);
	if (bytes == SOCKET_ERROR)
	{
		return MakeError(error::fail);
	}
	else if (bytes == 0)
	{
		return MakeError(error::connection_closed);
	}
	else
	{
		return bytes;
	}
#elif UT_UNIX
	ssize_t bytes = recv(GetPlatformSocket(), (char*)data, data_size, 0);
	if (bytes < 0)
	{
		return MakeError(Error(ConvertErrno(errno)));
	}
	else if (bytes == 0)
	{
		return MakeError(error::connection_closed);
	}
	else
	{
		return (int)bytes;
	}
#else
	#error ut::Socket::Recv() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Sends data
//    @param receiver - use corresponding client socket for servers, or self for clients
//    @param data - pointer to the first byte of the buffer for expected data
//    @param data_size - size of @data in bytes
//    @return - number of sent bytes
Result<int, Error> Socket::Send(const byte* data,
                                size_t data_size) const
{
#if UT_WINDOWS
	int bytes = send(GetPlatformSocket(), (char*)data, (int)data_size, 0);
	if (bytes == SOCKET_ERROR)
	{
		return MakeError(error::fail);
	}
	else
	{
		return bytes;
	}
#elif UT_UNIX
	ssize_t bytes = send(GetPlatformSocket(), (char*)data, data_size, 0);
	if (bytes < 0)
	{
		return MakeError(Error(ConvertErrno(errno)));
	}
	else
	{
		return (int)bytes;
	}
#else
	#error ut::Socket::Send() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Creates new platform @socket using @sock_addr_in address
//    @return - ut::Error if error occurred
Optional<Error> Socket::Create()
{
#if UT_WINDOWS
	// create socket
	socket = UT_INVALID_SOCKET;
	if ((socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == UT_INVALID_SOCKET)
	{
		return error::fail;
	}
	// set recv timeout
	uint32 timeout = UT_SOCKET_RECV_TIMEOUT_SEC* 1000;
	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != 0)
	{
		return error::fail;
	}
#elif UT_UNIX
	// create socket
	socket = UT_INVALID_SOCKET;
	if ((socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == UT_INVALID_SOCKET)
	{
		return Error(ConvertErrno(errno));
	}
	// set recv timeout
	struct timeval tv;
	tv.tv_sec = UT_SOCKET_RECV_TIMEOUT_SEC;  /* N Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval)) != 0)
	{
		return Error(ConvertErrno(errno));
	}
#else
	#error ut::Socket::Create() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns @socket member
socket_t Socket::GetPlatformSocket() const
{
	return socket;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//