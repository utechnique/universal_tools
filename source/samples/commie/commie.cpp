//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "commie.h"
#include "server.h"
#include "client.h"
#include "cfg.h"
#include "commands/message_cmd.h"
//----------------------------------------------------------------------------//
#if COMMIE_DESKTOP
#    include "ui/desktop_ui.h"
#else
	
#endif
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Choose UI type according to the current platform.
#if COMMIE_DESKTOP
typedef DesktopUI PlatformUI;
#else
	
#endif
//----------------------------------------------------------------------------//
// Job for the network routine.
class HostJob : public ut::Job
{
public:
	// Constructor, reference to the network node must be provided.
	HostJob(ut::net::Host& in_host) : host(in_host)
	{}

	// Launches host and waits till it works.
	void Execute()
	{
		ut::Optional<ut::Error> host_execution_error = host.Run();
		if (host_execution_error)
		{
			ut::log << host_execution_error.Get().GetDesc() << ut::cret;
		}
	}

private:
	// reference to the network node (client or server)
	ut::net::Host& host;
};

//----------------------------------------------------------------------------//
// Constructor
// Loads cfg file, opens console, creates and launches UI and host node.
Application::Application()
{
	// load configuration from file
	ut::Optional<ut::Error> load_cfg_error = LoadCfg();
	if (load_cfg_error)
	{
		throw load_cfg_error;
	}

	// open console
	if (cfg.console)
	{
		ut::console.Open();
	}

	// create the ui
	ui = ut::MakeUnique<PlatformUI>(*this);
	BindUserInterfaceSignals();

	// create host
	if (cfg.server_mode)
	{
		Server* server = new Server(ut::net::HostAddress(ut::String(), cfg.address.port),
		                            cfg.authorization_password);
		BindServerSignals(*server);
		host = ut::UniquePtr<Server>(server);
	}
	else
	{
		// fill authorization data
		ClientInitializationData ini_data;
		ini_data.name = cfg.name;
		ini_data.password = cfg.authorization_password;
		ini_data.encryption_key = cfg.encryption_key;

		// create client
		Client* client = new Client(ut::net::HostAddress(cfg.address.ip, cfg.address.port),
		                            ui.GetRef(),
		                            ini_data);
		BindClientSignals(*client);
		host = ut::UniquePtr<Client>(client);
	}

	// bind signals
	BindHostSignals(host.GetRef());

	// run host thread
	ut::UniquePtr<ut::Job> host_job(new HostJob(host.GetRef()));
	host_thread = ut::MakeUnique<ut::Thread>(ut::Move(host_job));

	// run user interface
	ui->Run();

	// stop host
	ut::Optional<ut::Error> shutdown_error = host->ShutDown();
	if (shutdown_error)
	{
		throw shutdown_error;
	}

	// wait for host thread
	host_thread.Delete();
}

//----------------------------------------------------------------------------->
// Destructor
// Updates configuration file and closes console.
Application::~Application()
{
	// update ui configuration
	ui->SyncCfg(cfg);

	// update configuration
	ut::log << "Updating config file before exit.." << ut::cret;
	ut::Optional<ut::Error> update_cfg_error = SaveCfgFile();
	if (update_cfg_error)
	{
		ut::log << update_cfg_error.Get().GetDesc() << ut::cret;
	}

	// close the console
	ut::console.Close();
}

//----------------------------------------------------------------------------->
// Returns a copy of the configuration object.
Configuration Application::GetCfg()
{
	return cfg;
}

//----------------------------------------------------------------------------->
// Loads and deserializes config file from the default cfg file path.
//    @return - error if failed.
ut::Optional<ut::Error> Application::LoadCfg()
{
	ut::File cfg_file;
	ut::Optional<ut::Error> open_cfg_error = cfg_file.Open(skCfgFileName,
	                                                       ut::File::Access::read);
	if (open_cfg_error)
	{
		// create default cfg file if it doesn't exist
		if (open_cfg_error.Get().GetCode() == ut::error::no_such_file)
		{
			ut::log << "No cfg file was found, creating a new one.." << ut::cret;
			ut::Optional<ut::Error> update_cfg_error = SaveCfgFile();
			if (update_cfg_error)
			{
				ut::log << "Failed to update config file:" << ut::cret;
				ut::log << update_cfg_error.Get().GetDesc() << ut::cret;
			}
		}
		else
		{
			// some bad error happened
			ut::log << "Failed to load cfg file:" << ut::cret;
			ut::log << open_cfg_error.Get().GetDesc() << ut::cret;
		}
	}
	else
	{
		// deserialize cfg from the text document
		ut::JsonDoc json_doc;
		ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(cfg);
		cfg_file >> json_doc >> cfg_snapshot;
		cfg_file.Close();
		ut::log << "Loaded config file " << skCfgFileName << "." << ut::cret;
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Updates current configuration object with provided serialized data.
//    @param doc - text document to be deserialized into current cfg object.
//    @return - error if failed
ut::Optional<ut::Error> Application::UpdateCfg(ut::text::Document& doc)
{
	ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(cfg);
	doc >> cfg_snapshot;
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Serializes and saves configuration object to the default cfg file path.
//    @return - error if failed
ut::Optional<ut::Error> Application::SaveCfgFile()
{
	ut::File cfg_file;
	ut::Optional<ut::Error> open_error = cfg_file.Open(skCfgFileName,
	                                                   ut::File::Access::write);
	if (open_error)
	{
		return open_error;
	}
	else
	{
		ut::JsonDoc json_doc;
		ut::meta::Snapshot cfg_snapshot = ut::meta::Snapshot::Capture(cfg);
		cfg_file << (json_doc << cfg_snapshot);
		cfg_file.Close();
		ut::log << "Cfg file was updated." << ut::cret;
	}

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Sends provided network command.
//    @param command - r-value reference to the unique network command.
//    @return - error if failed.
ut::Optional<ut::Error> Application::SendCommand(ut::UniquePtr<ut::net::Command> command)
{
	return host->SendCommand(Move(command));
}

//----------------------------------------------------------------------------->
// Binds signals that are related to UI.
void Application::BindUserInterfaceSignals()
{
	UT_ASSERT(ui != nullptr);

	// send message
	auto function = ut::MemberFunction<Application, void(const ut::String&,
	                                                    const ut::net::HostAddress&,
	                                                    bool)>(this, &Application::SendMessageEvent);
	ui->message_sent.Connect(ut::Move(function));
}

//----------------------------------------------------------------------------->
// Slot for event that sends a message.
//    @param message - text of the message to be sent.
//    @param address - destination network address.
//    @param encrypted - 'true' if message must be encrypted.
void Application::SendMessageEvent(const ut::String& message,
                                   const ut::net::HostAddress& address,
                                   bool encrypted)
{
	UT_ASSERT(host != nullptr);

	// echo
	if(address.IsValid()) // if address is invalid - it will be sent to all clients
	{                     // by server, so we don't need to echo it in that case.
		ut::String echo = commie::MessageCmd::ApplyTags(message,
														encrypted,
														address.IsValid(),
														cfg.name);
		ui->DisplayText(echo);
	}

	// send command
	ut::UniquePtr<ut::net::Command> cmd(new commie::MessageCmd(message,
	                                                           address,
	                                                           encrypted,
	                                                           cfg.encryption_key));
	host->SendCommand(ut::Move(cmd));
}

//----------------------------------------------------------------------------->
// Binds signals that are related to host node.
//    @param host - reference to the host object.
void Application::BindHostSignals(ut::net::Host& host)
{
	// launched
	auto host_launched = ut::MemberFunction<Application, void(const ut::net::HostAddress&)>(this, &Application::HostLaunchedEvent);
	host.launched.Connect(ut::Move(host_launched));

	// stopped
	auto host_stopped = ut::MemberFunction<Application, void()>(this, &Application::HostStoppedEvent);
	host.stopped.Connect(ut::Move(host_stopped));

	// command failed
	auto cmd_failed = ut::MemberFunction<Application, void(const ut::Error& error)>(this, &Application::CmdFailedEvent);
	host.command_failed.Connect(ut::Move(cmd_failed));

	// command failed
	auto connection_closed = ut::MemberFunction<Application, void(const ut::net::HostAddress&)>(this, &Application::ConnectionClosedEvent);
	host.connection_closed.Connect(ut::Move(connection_closed));
}

//----------------------------------------------------------------------------->
// Slot for event that launches a node.
//    @param address - network address of the node.
void Application::HostLaunchedEvent(const ut::net::HostAddress& address)
{
	UT_ASSERT(ui != nullptr);
	ut::String message = "Host launched (IP:";
	message += address.ip.Length() == 0 ? ut::String("ANY") : address.ip;
	message += ut::String(", port:") + ut::Print(address.port) + ")." + ut::cret;
	ut::log << message;
	ui->DisplayText(message);
}

//----------------------------------------------------------------------------->
// Slot for an event when host was stopped.
void Application::HostStoppedEvent()
{
	UT_ASSERT(ui != nullptr);
	ut::String message = ut::String("Host stopped.") + ut::cret;
	ut::log << message;
	ui->DisplayText(message);
}

//----------------------------------------------------------------------------->
// Slot for an event when some command failed to execute.
//    @param error - const reference to the error object.
void Application::CmdFailedEvent(const ut::Error& error)
{
	UT_ASSERT(ui != nullptr);
	ut::String message = ut::String("Command failed.") + ut::cret;
	message += error.GetDesc() + ut::cret;
	ut::log << message;
	ui->DisplayText(message);
}

//----------------------------------------------------------------------------->
// Slot for an event when a network connection was closed.
//    @param address - network address of this connection.
void Application::ConnectionClosedEvent(const ut::net::HostAddress& address)
{
	UT_ASSERT(ui != nullptr);
	ut::String message = "Connection closed (";
	message += address.ToString() + ")." + ut::cret;
	ut::log << message;
	ui->DisplayText(message);
}

//----------------------------------------------------------------------------->
// Binds signals that are specific to server type of node.
//    @param server - reference to the server object.
void Application::BindServerSignals(Server& server)
{
	// client accepted
	auto client_accepted = ut::MemberFunction<Application, void(const ut::net::HostAddress&)>
		(this, &Application::ClientAcceptedEvent);
	server.client_accepted.Connect(ut::Move(client_accepted));
}

//----------------------------------------------------------------------------->
// Slot for an event when a new client connected to the server.
//    @param address - network address of this client.
void Application::ClientAcceptedEvent(const ut::net::HostAddress& address)
{
	UT_ASSERT(ui != nullptr);
	ut::String message = "Client accepted (";
	message += address.ToString() + ")." + ut::cret;
	ut::log << message;
	ui->DisplayText(message);
}

//----------------------------------------------------------------------------->
// Binds signals that are specific to client type of node.
//    @param client - reference to the client object.
void Application::BindClientSignals(Client& client)
{
	// connection failed
	auto connection_failed = ut::MemberFunction<Application, void(const ut::net::HostAddress&)>
		(this, &Application::ConnectionFailedEvent);
	client.connection_failed.Connect(ut::Move(connection_failed));

	// connected to server
	auto connected_to_server = ut::MemberFunction<Application, void(const ut::net::HostAddress&)>
		(this, &Application::ConnectedToServerEvent);
	client.connected_to_server.Connect(ut::Move(connected_to_server));
}

//----------------------------------------------------------------------------->
// Slot for an event when connection (to server) attempt has failed.
//    @param address - network address of the server.
void Application::ConnectionFailedEvent(const ut::net::HostAddress& address)
{
	UT_ASSERT(ui != nullptr);
	ut::String message = "Connection failed (";
	message += address.ToString() + ")." + ut::cret;
	ut::log << message;
	ui->DisplayText(message);
}

//----------------------------------------------------------------------------->
// Slot for an event when connection (to server) attempt succeeded.
//    @param address - network address of the server.
void Application::ConnectedToServerEvent(const ut::net::HostAddress& address)
{
	UT_ASSERT(ui != nullptr);
	ut::String message = "Connected to server (";
	message += address.ToString() + ")." + ut::cret;
	ut::log << message;
	ui->DisplayText(message);
}

//----------------------------------------------------------------------------->
// Default local file path for the configuration file.
const ut::String Application::skCfgFileName = "cfg.json";

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//