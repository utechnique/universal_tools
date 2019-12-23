//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_host.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// ut::net::Server implements server side of the client-server pattern.
class Server : public Host
{
public:
	// Constructor.
	//    @param address - address of the clients that can be accepted, leave
	//                     it empty to allow any connection.
	//    @param in_accept_timeout_ms - server gives up listening after this
	//                                  interval of time (in milliseconds),
	//                                  then tries again in a loop.
	Server(const HostAddress& address, uint in_accept_timeout_ms = 100);

	// Destructor.
	virtual ~Server();

	// Creates a new connection using provided socket. Must be overloaded by
	// child class.
	//    @param socket - socket of the connection to be created.
	//    @return - unique pointer to the new connection.
	virtual UniquePtr<Connection> CreateConnection(UniquePtr<Socket> socket) = 0;

	// Listens for the new clients. If client was accepted - creates new connection.
	//    @return - error if failed.
	Optional<Error> Run();

	// Executes provided command.
	//    @param cmd - reference to the command to be executed.
	//    @param connection - reference to the connection containing the command.
	//    @return - error if function failed.
	Optional<Error> ExecuteCommand(Command& cmd, Connection& connection);

	// server-specific signals
	Signal<void(const HostAddress& address)> client_accepted;

private:
	// server gives up listening after this
	// interval of time (in milliseconds),
	// then tries again in a loop.
	uint accept_timeout_ms;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//