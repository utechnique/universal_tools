//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie_common.h"
#include "../client_list.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::ClientListCmd is a network command to request information about
// current connections of the server node.
class ClientListCmd : public ut::net::Command
{
public:
	// Constructor
	ClientListCmd();

	// Copy constructor
	ClientListCmd(const ClientListCmd& copy);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Serialization
	void Reflect(ut::meta::Snapshot& snapshot);

	// Executes command if received by client.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	ut::Optional<ut::Error> ExecuteOnClient(ut::net::Connection& connection);

	// Executes command if received by server.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	ut::Optional<ut::Error> ExecuteOnServer(ut::net::Connection& connection);

private:
	ClientList list;
};


//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//