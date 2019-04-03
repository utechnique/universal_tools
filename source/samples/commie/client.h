//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ui/ui.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::ClientInitializationData is a structure to contain data that is
// needed to establish correct connection.
struct ClientInitializationData
{
	// Constructor.
	ClientInitializationData();

	// Copy constructor.
	ClientInitializationData(const ClientInitializationData& copy);

	// authorization password.
	ut::String password;

	// name of the client.
	ut::String name;

	// Cipher key to encrypt messages.
	ut::String encryption_key;
};

//----------------------------------------------------------------------------//
// commie::Client is a class implementing client-side of the application.
class Client : public ut::net::Client
{
public:
	// Constructor.
	//    @param address - address of the server.
	//    @param user_interface - reference to the UI object.
	//    @param in_initialization_data - const reference to the data that is
	//                                    needed to initialize client node.
	Client(const ut::net::HostAddress& address,
	       UI& user_interface,
	       const ClientInitializationData& in_initialization_data);

	// Creates connection with specific communication protocol.
	ut::UniquePtr<ut::net::Connection> CreateConnection(ut::RValRef<ut::net::SocketPtr>::Type socket);

	// Returns a reference to the UI object.
	UI& GetUI();

	// Tries to connect to the server and establishes a connection if succeeded.
	// If connection was lost, tries to connect again.
	//    @return - error if failed.
	ut::Optional<ut::Error> Run();

	// Asks UI to update it's client list widget with provided data.
	//    @param list - reference to the array of commie::ClientInfo elements.
	//    @return - error if failed.
	ut::Optional<ut::Error> UpdateClientList(const ClientList& list);

	// Returns a string containing cipher key for encrypting outgoing messages.
	ut::String GetEncryptionKey() const;

private:
	// Time interval before next connection attempt.
	static const ut::uint32 skConnectionAttemptFreq; // in milliseconds

	// Frequency of the client-list update requests.
	static const ut::uint32 skUpdateClientListFreq; // in milliseconds

	// Reference to the UI object.
	UI& ui;

	// Data that is needed to establish correct connection.
	ClientInitializationData initialization_data;

	// Thread that is sending client-list-update commands to the server
	ut::UniquePtr<ut::Thread> clientlist_thread;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//