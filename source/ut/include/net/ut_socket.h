//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "pointers/ut_unique_ptr.h"
#include "error/ut_error.h"
#include "net/ut_socket_system.h"
#include "net/ut_host_address.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// class ut::Socket is a high-level wrapper for native os sockets. It's highly
// recommended to prefer SocketPtr rather than Socket, as it's a bit tricky to
// perform copying on sockets. For example, imagine ut::Socket opens new socket
// with the same address as the source's one while copying, but if source
// started listening or something - this behaviour would be lost in new socket.
// Thus copying is forbidden. Use unique sockets and avoid copying.
class Socket : public NonCopyable
{
public:
	// Default constructor, calls ut::Socket::Create() internally
	Socket();

	// Constructor
	//    @param ip - can be ipv4/ipv6 address or domain name
	//    @param port - port, must be > 0
	Socket(const String& ip, int port);

	// Move constructor, moves socket and address
#if CPP_STANDARD >= 2011
	Socket(Socket && copy);
#endif

	// Move operator, moves socket and address
#if CPP_STANDARD >= 2011
	Socket& operator = (Socket && copy);
#endif

	// Destructor, closes @socket, and optionally calls
	// ut::Socket::ShutDown() (Linux)
	~Socket();

	// Sets socket address
	//    @param ip - can be ipv4/ipv6 address or domain name
	//    @param port - port, must be > 0
	void SetAddr(const String& ip, int port);

	// Sets socket address
	//    @param address - ut::HostAddress object
	void SetAddr(const HostAddress& address);

	// Returns socket address or ut::Error if error occurred
	Result<HostAddress, Error> GetAddr() const;

	// Returns local socket address (for clients) or ut::Error
	// if error occurred
	Result<HostAddress, Error> GetLocalAddr() const;

	// Binds socket
	//    @return - ut::Error if error occurred
	Optional<Error> Bind() const;

	// Listens sockets
	//    @return - ut::Error if error occurred
	Optional<Error> Listen() const;

	// Accepts socket
	//    @return - connected socket pointer if success, or error otherwise
	Result<UniquePtr<Socket>, Error> Accept() const;

	// Poll socket events
	//    @param timeout_ms - timieout in milliseconds
	//    @return - ut::Error if error occurred
	Optional<Error> Poll(uint32 timeout_ms) const;

	// Shutdown
	//    @return - ut::Error if error occurred
	Optional<Error> ShutDown() const;

	// Connects to the listening socket
	//    @return - ut::Error if error occurred
	Optional<Error> Connect() const;

	// Receives data
	//    @param data - pointer to the first byte of data to be sent
	//    @param data_size - size of @data in bytes
	//    @return - number of received bytes, or ut::Error if error occurred
	Result<int, Error> Recv(byte* data,
	                        size_t data_size) const;

	// Sends data
	//    @param data - pointer to the first byte of the buffer of expected data
	//    @param data_size - size of @data in bytes
	//    @return - number of sent bytes, or ut::Error if error occurred
	Result<int, Error> Send(const byte* data,
	                        size_t data_size) const;

	// Receives single element
	//    @param element - received data will be written to this object
	//    @return - number of received bytes, or ut::Error if error occurred
	template<typename T>
	Result<int, Error> RecvElement(T& element) const
	{
		return Recv((byte*)&element, sizeof(T));
	}

	// Receives array of elements, note that one extra uint32 element is
	// received to know the size of the array. Use this function only if
	// sender socket sent the array by calling
	// ut::Socket::SendArray() function.
	//    @param arr - received data will be written to this array
	//    @return - number of received bytes, or ut::Error if error occurred
	template<typename T>
	Result<int, Error> RecvArray(Array<T>& arr) const
	{
		// receive the number of elemts in the array
		uint32 arr_size;
		Result<int, Error> recv_size_result = RecvElement<uint32>(arr_size);
		if (!recv_size_result)
		{
			return MakeError(recv_size_result.MoveAlt());
		}

		// resize array
		arr.Resize((size_t)arr_size);

		// receive array
		Result<int, Error> recv_arr_result = Recv((byte*)arr.GetAddress(),
			                                        arr_size * sizeof(T));
		if (!recv_arr_result)
		{
			return MakeError(recv_arr_result.MoveAlt());
		}

		// calculate received bytes
		return recv_size_result.GetResult() + recv_arr_result.GetResult();
	}

	// Sends single element
	//    @param element - object to be sent
	//    @return - number of sent bytes, or ut::Error if error occurred
	template<typename T>
	Result<int, Error> SendElement(const T& element) const
	{
		return Send((const byte*)&element, sizeof(T));
	}

	// Sends array of elements, note that one extra uint32 element is sent to
	// provide information about array size to the receiver. Use this
	// function only if receiver socket is expected to receive the array by
	// calling ut::Socket::RecvArray() function.
	//    @param arr - array to be sent
	//    @return - number of sent bytes, or ut::Error if error occurred
	template<typename T>
	Result<int, Error> SendArray(const Array<T>& arr) const
	{
		uint32 arr_size = (uint32)arr.GetNum();
		Result<int, Error> send_size_result = SendElement<uint32>(arr_size);
		if (send_size_result)
		{
			return Send((const byte*)arr.GetAddress(), arr_size* sizeof(T));
		}
		else
		{
			return MakeError(send_size_result.GetAlt());
		}
	}

private:
	// Private constructor, for internal use only.
	// Arguments are native platform socket and socket address.
	Socket(socket_t in_socket, sockaddr_in *in_sockaddr);

	// Creates new platform @socket using @sock_addr_in address
	//    @return - ut::Error if error occurred
	Optional<Error> Create();

	// Returns @socket member
	socket_t GetPlatformSocket() const;

	// Native socket handle
	socket_t socket;

	// Native socket address
	sockaddr_in sock_addr_in;
	
private:
	// Don't use this reference. It's needed to make linker mark
	// ut::socket_system global variable relevent if at least one
	// ut::Socket object was created. As a result, platform-specific
	// socket libraries will be loaded only if there is a real need of them.
	static const SocketSystem& skSystem;
};

// Shorter type name of the UniquePtr<class Socket>
typedef UniquePtr<class Socket> SocketPtr;

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//