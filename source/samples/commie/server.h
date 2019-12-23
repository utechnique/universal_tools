//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ui/ui.h"
#include "client_list.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::Server is a class implementing server-side of the application.
class Server : public ut::net::Server
{
public:
	// Constructor
	//    @param address - address of possible incoming connections.
	//    @param authorization_password - password to authorize clients.
	Server(const ut::net::HostAddress& address,
	       const ut::String& authorization_password);

	// Creates connection with specific communication protocol.
	ut::UniquePtr<ut::net::Connection> CreateConnection(ut::UniquePtr<ut::net::Socket> socket);

	// Returns authorization password.
	ut::String GetPassword() const;

	// Returns a list of current connections.
	ClientList GetClientList();

private:
	// Authorization password.
	const ut::String password;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//