//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_host.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// ut::net::Client implements client side of the client-server pattern.
class Client : public Host
{
public:
	// Constructor
	//    @param address - address of the server.
	//    @param conn_interval_ms - connection attempt frequency
	//                              in millisecons.
	Client(const HostAddress& address, uint32 conn_interval_ms = 0);

	// Destructor
	virtual ~Client();

	// Creates a new connection using provided socket. Must be overloaded by
	// child class.
	//    @param socket - socket of the connection to be created.
	//    @return - unique pointer to the new connection.
	virtual UniquePtr<Connection> CreateConnection(UniquePtr<Socket> socket) = 0;

	// Tries to connect to the server and establishes a connection if succeeded.
	// If connection was lost, tries to connect again.
	//    @return - error if failed.
	Optional<Error> Run();

	// Executes provided command.
	//    @param cmd - reference to the command to be executed.
	//    @param connection - reference to the connection containing the command.
	//    @return - error if function failed.
	Optional<Error> ExecuteCommand(Command& cmd, Connection& connection);

	// client-specific signals
	Signal<void(const HostAddress& address)> connection_failed;
	Signal<void(const HostAddress& address)> connected_to_server;

private:
	// connection attempt frequency in millisecons
	uint32 connection_interval_ms;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//