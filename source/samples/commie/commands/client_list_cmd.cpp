//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "../client.h"
#include "../server.h"
#include "../connection.h"
#include "client_list_cmd.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ut::net::Command, commie::ClientListCmd, "client_list")
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Constructor
ClientListCmd::ClientListCmd()
{ }

// Copy constructor
ClientListCmd::ClientListCmd(const ClientListCmd& copy) : list(copy.list)
{ }

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& ClientListCmd::Identify() const
{
	return ut::Identify(this);
}

// Serialization
void ClientListCmd::Serialize(ut::MetaStream& stream)
{
	stream << list;
}

// Executes command if received by client.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> ClientListCmd::ExecuteOnClient(ut::net::Connection& connection)
{
	// update client list in ui
	commie::Client& client = static_cast<commie::Client&>(connection.GetHost());
	ut::Optional<ut::Error> update_error = client.UpdateClientList(list);
	if (update_error)
	{
		ut::log << update_error.Get().GetDesc() << ut::cret;
	}

	// success
	return ut::Optional<ut::Error>();
}

// Executes command if received by server.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> ClientListCmd::ExecuteOnServer(ut::net::Connection& connection)
{
	// get client list
	commie::Server& server = static_cast<commie::Server&>(connection.GetHost());
	list = server.GetClientList();

	// send client list back to the sender
	ut::UniquePtr<ut::net::Command> copy(new ClientListCmd(*this));
	return connection.GetHost().SendCommand(Move(copy), connection.GetAddress());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//