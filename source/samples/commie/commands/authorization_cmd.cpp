//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "authorization_cmd.h"
#include "message_cmd.h"
#include "../server.h"
#include "../connection.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ut::net::Command, commie::ClientAuthorizationCmd, "client_authorization")
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Callback to shutdown connection after some command was sent. Is used only
// by server node.
ut::Optional<ut::Error> ShutDownConnection(ut::net::Connection& connection)
{
	return ut::Error(ut::error::authorization);
}

//----------------------------------------------------------------------------//
// Default constructor
ClientAuthorizationCmd::ClientAuthorizationCmd()
{ }

// Constructor
//    @param in_password - authorization password, connection will be
//                         aborted if this password is invalid.
//    @param in_client_name - name of the client that is being authorized.
ClientAuthorizationCmd::ClientAuthorizationCmd(const ut::String& in_password,
                                               const ut::String& in_client_name) : password(in_password)
                                                                                 , client_name(in_client_name)
{ }

// Copy constructor
ClientAuthorizationCmd::ClientAuthorizationCmd(const ClientAuthorizationCmd& copy)
{ }

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& ClientAuthorizationCmd::Identify() const
{
	return ut::Identify(this);
}

// Serialization
void ClientAuthorizationCmd::Serialize(ut::MetaStream& stream)
{
	stream << password;
	stream << client_name;
}

// Executes command if received by client.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> ClientAuthorizationCmd::ExecuteOnClient(ut::net::Connection& connection)
{
	// success
	return ut::Optional<ut::Error>();
}

// Executes command if received by server.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> ClientAuthorizationCmd::ExecuteOnServer(ut::net::Connection& connection)
{
	// convert ut::Server to commie::Server
	commie::Server& server = static_cast<commie::Server&>(connection.GetHost());

	// check if password is correct
	if (password == server.GetPassword())
	{
		// set client name
		commie::Connection& commie_connection = static_cast<commie::Connection&>(connection);
		commie_connection.SetClientName(client_name);
	}
	else
	{
		// send error message and shutdown connection after it was sent
		ut::String msg = "Authorization failed: Invalid Password.";
		ut::UniquePtr<ut::net::Command> message_cmd(new commie::MessageCmd(msg));
		message_cmd->sent.Connect(ShutDownConnection);
		server.SendCommand(ut::Move(message_cmd), connection.GetAddress());
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//