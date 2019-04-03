//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "commie_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::Connection is a class that implements communication protocol for
// all commie connections (both server and client side)
class Connection : public ut::net::Connection
{
public:
	// Constructor
	//    @param owner - server or client holding current connection.
	//    @param socket - socket of the current connection.
	//    @param in_actions - array of network actions describing communication
	//                        protocol.
	Connection(ut::net::Host& owner,
	           ut::RValRef< ut::UniquePtr<ut::net::Socket> >::Type in_socket,
	           ut::RValRef< ut::Array< ut::UniquePtr<ut::net::Action> > >::Type in_actions);

	// Returns a name of the client that is associated with current connection.
	// Usefull only for the server node.
	ut::String GetClientName() const;

	// Assign a name of the client that is associated with current connection.
	// Usefull only for the server node.
	void SetClientName(const ut::String& name);

private:
	// Name of the client that is associated with current connection.
	ut::String client_name;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//