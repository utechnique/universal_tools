//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_host_address.h"
#include "net/ut_socket_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// Default constructor, @ip is empty, and @port is '-1'
HostAddress::HostAddress() : port(-1)
{}

// Constructor
//    @param ip - can be ipv4/6 address or domain name
//    @param port - port, will be converted to signed integer internally
HostAddress::HostAddress(const String& in_ip, int in_port) : ip(in_ip)
                                                           , port((int)in_port)
{}

// Copy constructor, @ip and @port are copied
HostAddress::HostAddress(const HostAddress& copy) : ip(copy.ip), port(copy.port)
{}

// Move constructor, @ip is moved, @port is copied
#if CPP_STANDARD >= 2011
HostAddress::HostAddress(HostAddress && copy) : ip(Move(copy.ip)), port(copy.port)
{

}
#endif

// Assignment operator, both @ip and @port are copied
HostAddress& HostAddress::operator = (const HostAddress& copy)
{
	ip = copy.ip;
	port = copy.port;
	return *this;
}

// Move operator, @ip is moved, @port is copied
#if CPP_STANDARD >= 2011
HostAddress& HostAddress::operator = (HostAddress && copy)
{
	ip = Move(copy.ip);
	port = copy.port;
	return *this;
}
#endif

// Comparison operator
//    @param address - address to compare with
bool HostAddress::operator == (const HostAddress& address) const
{
	// check ip
	if (ip != address.ip)
	{
		return false;
	}

	// check port
	if (port != address.port)
	{
		return false;
	}

	// matches
	return true;
}

// Comparison operator
//    @param address - address to compare with
bool HostAddress::operator != (const HostAddress& address) const
{
	return port != address.port || ip != address.ip;
}

// Returns 'true' if address was correctly initialized.
bool HostAddress::IsValid() const
{
	// check port
	if (port < 0)
	{
		return false;
	}

	// check ip
	if (ip.Length() == 0)
	{
		return false;
	}

	// valid
	return true;
}

// Converts address to string
String HostAddress::ToString() const
{
	return ip + ":" + Print(port);
}

//----------------------------------------------------------------------------//
// Returns network name of the current device or ut::Error if error occurred
Result<String, Error> GetHostName()
{
	// ut::socket_system must be created for this function 
	volatile const SocketSystem& socksys_ref = socket_system;

	char hname[128];
#if UT_WINDOWS
	if (gethostname(hname, 128))
	{
		return MakeError(error::fail);
	}
	else
	{
		return String(hname);
	}
#elif UT_UNIX
	if (gethostname(hname, 128))
	{
		return MakeError(Error(ConvertErrno(errno)));
	}
	else
	{
		return String(hname);
	}
#else
#error ut::GetHostName() is not implemented
#endif
}

//----------------------------------------------------------------------------//

Result<String, Error> GetHostByName(const String& hostname)
{
	// ut::socket_system must be created for this function 
	volatile const SocketSystem& socksys_ref = socket_system;

#if UT_WINDOWS
	hostent* htn = gethostbyname(hostname.GetAddress());
	if (htn == nullptr)
	{
		return MakeError(error::fail);
	}

	const char* cs = inet_ntoa(**(in_addr**)htn->h_addr_list);
	if (cs == nullptr)
	{
		return MakeError(error::fail);
	}
	else
	{
		return String(cs);
	}
#elif UT_UNIX
	hostent* htn = gethostbyname(hostname.GetAddress());
	if (htn == nullptr)
	{
		return MakeError(ConvertErrno(h_errno));
	}

	const char* cs = inet_ntoa(**(in_addr**)htn->h_addr_list);
	if (cs == nullptr)
	{
		return MakeError(error::fail);
	}
	else
	{
		return String(cs);
	}
#else
#error ut::GetHostByName() is not implemented
#endif
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//