//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "client.h"
#include "connection.h"
#include "commands/authorization_cmd.h"
#include "commands/idle_cmd.h"
#include "commands/client_list_cmd.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Time interval before next connection attempt.
const ut::uint32 Client::skConnectionAttemptFreq = 1500;

// Frequency of the client-list update requests.
const ut::uint32 Client::skUpdateClientListFreq = 5000;

//----------------------------------------------------------------------------//
// Job that is sending client-list-update commands to the server.
class ClientListJob : public ut::Job
{
public:
	// Constructor
	//    @param in_client - reference to the client object.
	//    @param in_ui - reference to the UI object.
	//    @param in_frequency_ms - frequency of requests in milliseconds.
	ClientListJob(commie::Client& in_client,
	              UI& in_ui,
	              ut::uint32 in_frequency_ms) : client(in_client)
	                                          , ui(in_ui)
	                                          , frequency_ms(in_frequency_ms)
	{ }

	// Thread routine.
	void Execute()
	{
		while (!exit_request.Get())
		{
			if (client.IsActive())
			{
				ut::UniquePtr<ut::net::Command> cmd(new ClientListCmd);
				client.SendCommand(ut::Move(cmd));
			}
			
			// wait, but be ready to exit
			ut::uint32 sleep_quantum = 200;
			ut::uint32 wait_step = frequency_ms / sleep_quantum;
			for (ut::uint32 i = 0; i < wait_step; i++)
			{
				ut::Sleep(sleep_quantum);

				if (exit_request.Get())
				{
					return;
				}
			}
		}
	}

private:
	// reference to the client object.
	commie::Client& client;

	// reference to the UI object.
	UI& ui;

	// frequency of requests in milliseconds.
	ut::uint32 frequency_ms;
};

//----------------------------------------------------------------------------//
// Constructor.
//    @param address - address of the server.
//    @param user_interface - reference to the UI object.
//    @param in_initialization_data - const reference to the data that is
//                                    needed to initialize client node.
Client::Client(const ut::net::HostAddress& address,
               UI& user_interface,
               const ClientInitializationData& in_init_data) : ut::net::Client(address, skConnectionAttemptFreq)
                                                             , ui(user_interface)
                                                             , initialization_data(in_init_data)
{ }

// Creates connection with specific communication protocol.
ut::UniquePtr<ut::net::Connection> Client::CreateConnection(ut::RValRef<ut::net::SocketPtr>::Type socket)
{
	// action sequence
	ut::Array< ut::UniquePtr<ut::net::Action> > actions;

	// idle command
	ut::UniquePtr<ut::net::Command> idle_cmd(new IdleCmd(IdleCmd::skDefaultFrequency));

	// authorization command
	ut::UniquePtr<ut::net::Command> client_authorization_cmd(new ClientAuthorizationCmd(initialization_data.password,
	                                                                                    initialization_data.name));
	actions.Add(new ut::net::SendCommand(Move(client_authorization_cmd)));

	// main loop
	ut::Array< ut::UniquePtr<ut::net::Action> > looped_actions;
	looped_actions.Add(new ut::net::ReceiveCommand());
	looped_actions.Add(new ut::net::SendCommandFromStack(Move(idle_cmd)));
	actions.Add(new ut::net::Loop(ut::Move(looped_actions)));

	// create new connection
	ut::UniquePtr<ut::net::Connection> connection(new Connection(*this, Move(socket), Move(actions)));

	// move connection
	return Move(connection);
}

// Returns a reference to the UI object.
UI& Client::GetUI()
{
	return ui;
}

// Tries to connect to the server and establishes a connection if succeeded.
// If connection was lost, tries to connect again.
//    @return - error if failed.
ut::Optional<ut::Error> Client::Run()
{
	// run client-list update thread
	ut::UniquePtr<ut::Job> job(new ClientListJob(*this, ui, skUpdateClientListFreq));
	clientlist_thread = new ut::Thread(ut::Move(job));

	// run client
	return ut::net::Client::Run();
}

// Asks UI to update it's client list widget with provided data.
//    @param list - reference to the array of commie::ClientInfo elements.
//    @return - error if failed.
ut::Optional<ut::Error> Client::UpdateClientList(const ClientList& list)
{
	return ui.UpdateClientList(list);
}

// Returns a string containing cipher key for encrypting outgoing messages.
ut::String Client::GetEncryptionKey() const
{
	return initialization_data.encryption_key;
}

//----------------------------------------------------------------------------//
// Constructor.
ClientInitializationData::ClientInitializationData()
{ }

// Copy constructor.
ClientInitializationData::ClientInitializationData(const ClientInitializationData& copy)
	: password(copy.password)
	, name(copy.name)
	, encryption_key(copy.encryption_key)
{ }

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//