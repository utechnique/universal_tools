//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "commie_common.h"
#include "cfg.h"
#include "server.h"
#include "client.h"
#include "ui/ui.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::Application is the main class, it owns both host node and user
// interface.
class Application
{
public:
	// Constructor
	// Loads cfg file, opens console, creates and launches UI and host node.
	Application();

	// Destructor
	// Updates configuration file and closes console.
	~Application();

	// Returns a copy of the configuration object.
	Configuration GetCfg();

	// Loads and deserializes config file from the default cfg file path.
	//    @return - error if failed.
	ut::Optional<ut::Error> LoadCfg();

	// Updates current configuration object with provided serialized data.
	//    @param doc - text document to be deserialized into current cfg object.
	//    @return - error if failed
	ut::Optional<ut::Error> UpdateCfg(ut::text::Document& doc);

	// Serializes and saves configuration object to the default cfg file path.
	//    @return - error if failed
	ut::Optional<ut::Error> SaveCfgFile();

	// Sends provided network command.
	//    @param command - r-value reference to the unique network command.
	//    @return - error if failed.
	ut::Optional<ut::Error> SendCommand(ut::UniquePtr<ut::net::Command> command);

private:
	// Binds signals that are related to UI.
	void BindUserInterfaceSignals();

	// Slot for event that sends a message.
	//    @param message - text of the message to be sent.
	//    @param address - destination network address.
	//    @param encrypted - 'true' if message must be encrypted.
	void SendMessageEvent(const ut::String& message,
	                      const ut::net::HostAddress& address,
	                      bool encrypted);

	// Binds signals that are related to host node.
	//    @param host - reference to the host object.
	void BindHostSignals(ut::net::Host& host);

	// Slot for event that launches a node.
	//    @param address - network address of the node.
	void HostLaunchedEvent(const ut::net::HostAddress& address);

	// Slot for an event when host was stopped.
	void HostStoppedEvent();

	// Slot for an event when some command failed to execute.
	//    @param error - const reference to the error object.
	void CmdFailedEvent(const ut::Error& error);

	// Slot for an event when a network connection was closed.
	//    @param address - network address of this connection.
	void ConnectionClosedEvent(const ut::net::HostAddress& address);

	// Binds signals that are specific to server type of node.
	//    @param server - reference to the server object.
	void BindServerSignals(Server& server);

	// Slot for an event when a new client connected to the server.
	//    @param address - network address of this client.
	void ClientAcceptedEvent(const ut::net::HostAddress& address);

	// Binds signals that are specific to client type of node.
	//    @param client - reference to the client object.
	void BindClientSignals(Client& client);

	// Slot for an event when connection (to server) attempt has failed.
	//    @param address - network address of the server.
	void ConnectionFailedEvent(const ut::net::HostAddress& address);

	// Slot for an event when connection (to server) attempt succeeded.
	//    @param address - network address of the server.
	void ConnectedToServerEvent(const ut::net::HostAddress& address);

	// Set of options and preferences.
	Configuration cfg;

	// User interface.
	ut::UniquePtr<UI> ui;

	// Network node (client or server).
	ut::UniquePtr<ut::net::Host> host;

	// Thread for the network routine.
	ut::UniquePtr<ut::Thread> host_thread;

	// Default local file path for the configuration file.
	static const ut::String skCfgFileName;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//