//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "connection.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Constructor
//    @param owner - server or client holding current connection.
//    @param socket - socket of the current connection.
//    @param in_actions - array of network actions describing communication
//                        protocol.
Connection::Connection(ut::net::Host& owner,
                       ut::RValRef< ut::UniquePtr<ut::net::Socket> >::Type in_socket,
                       ut::RValRef< ut::Array< ut::UniquePtr<ut::net::Action> > >::Type in_actions)
	: ut::net::Connection(owner, ut::Move(in_socket), ut::Move(in_actions))
{

}

// Returns a name of the client that is associated with current connection.
// Usefull only for the server node.
ut::String Connection::GetClientName() const
{
	return client_name;
}

// Assign a name of the client that is associated with current connection.
// Usefull only for the server node.
void Connection::SetClientName(const ut::String& name)
{
	client_name = name;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//