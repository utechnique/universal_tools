//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "server.h"
#include "connection.h"
#include "commands/authorization_cmd.h"
#include "commands/idle_cmd.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Constructor
//    @param address - address of possible incoming connections.
//    @param authorization_password - password to authorize clients.
Server::Server(const ut::net::HostAddress& address,
               const ut::String& authorization_password) : ut::net::Server(address)
                                                         , password(authorization_password)
{ }

// Creates connection with specific communication protocol.
ut::UniquePtr<ut::net::Connection> Server::CreateConnection(ut::UniquePtr<ut::net::Socket> socket)
{
	// action sequence
	ut::Array< ut::UniquePtr<ut::net::Action> > actions;

	// idle command
	ut::UniquePtr<ut::net::Command> idle_cmd(new IdleCmd(IdleCmd::skDefaultFrequency));

	// authorization command
	actions.Add(ut::MakeUnique<ut::net::ReceiveCommand>(ut::GetPolymorphicName<ClientAuthorizationCmd>()));

	// main loop
	ut::Array< ut::UniquePtr<ut::net::Action> > looped_actions;
	looped_actions.Add(ut::MakeUnique<ut::net::SendCommandFromStack>(Move(idle_cmd)));
	looped_actions.Add(ut::MakeUnique<ut::net::ReceiveCommand>());
	actions.Add(ut::MakeUnique<ut::net::Loop>(ut::Move(looped_actions)));

	// create connection
	ut::UniquePtr<ut::net::Connection> connection(new Connection(*this, ut::Move(socket), ut::Move(actions)));

	// move connection
	return ut::Move(connection);
}

// Returns authorization password.
ut::String Server::GetPassword() const
{
	return password;
}

// Returns a list of current connections.
ClientList Server::GetClientList()
{
	ClientList list;

	// lock connections
	ut::ScopeSyncRWLock< ut::Array< ut::UniquePtr<ut::net::Connection> > > connections_scope_lock(connections, ut::RWLock::Access::write);
	ut::Array< ut::UniquePtr<ut::net::Connection> >& locked_connections = connections_scope_lock.Get();

	// iterate connections
	for (size_t i = 0; i < locked_connections.Count(); i++)
	{
		commie::Connection& commie_connection = static_cast<commie::Connection&>(locked_connections[i].GetRef());

		ClientInfo info;
		info.address = commie_connection.GetAddress();
		info.name = commie_connection.GetClientName();

		list.Add(info);
	}

	return list;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//