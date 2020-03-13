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
// ut::HostAddress contains full address to the node.
// @Ip and @port are separate variables, you can access them directly.
struct HostAddress
{
	// Default constructor, @ip is empty, and @port is '-1'
	HostAddress();

	// Constructor
	//    @param ip - can be ipv4/6 address or domain name
	//    @param port - port, will be converted to signed integer internally
	HostAddress(const String& in_ip, int in_port);

	// Comparison operator
	//    @param address - address to compare with
	bool operator == (const HostAddress& address) const;

	// Comparison operator
	//    @param address - address to compare with
	bool operator != (const HostAddress& address) const;

	// Returns 'true' if address was correctly initialized.
	bool IsValid() const;

	// Converts address to string
	String ToString() const;

	// ip address, can be ipv4/6 address or domain name
	String ip;

	// port, value '-1' means 'invalid'
	int port;
};

//----------------------------------------------------------------------------//
// Returns network name of the current device or ut::Error if error occurred
Result<String, Error> GetHostName();

//----------------------------------------------------------------------------//

Result<String, Error> GetHostByName(const String& hostname);

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//