//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::ClientAuthorizationCmd is a network command to athorize clients on
// the server side. If client didn't provide this command - connection will be
// aborted.
class ClientAuthorizationCmd : public ut::net::Command
{
public:
	// Default constructor
	ClientAuthorizationCmd();

	// Constructor
	//    @param in_password - authorization password, connection will be
	//                         aborted if this password is invalid.
	//    @param in_client_name - name of the client that is being authorized.
	ClientAuthorizationCmd(const ut::String& in_password,
	                       const ut::String& in_client_name);

	// Copy constructor
	ClientAuthorizationCmd(const ClientAuthorizationCmd& copy);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Serialization
	void Serialize(ut::MetaStream& stream);

	// Executes command if received by client.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	ut::Optional<ut::Error> ExecuteOnClient(ut::net::Connection& connection);

	// Executes command if received by server.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	ut::Optional<ut::Error> ExecuteOnServer(ut::net::Connection& connection);

private:
	ut::String password;
	ut::String client_name;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//