//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "pointers/ut_unique_ptr.h"
#include "error/ut_error.h"
#include "net/ut_connection.h"
#include "events/ut_signal.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// ut::net::Host is a base class for network nodes. ut::Server and ut::Client
// classes are inherited from ut::Host.
class Host : public ut::NonCopyable
{
public:
	// Constructor
	//    @param in_address - network address of the node.
	Host(const HostAddress& in_address);

	// Destructor
	virtual ~Host();

	// Creates a new connection using provided socket.
	//    @param socket - socket of the connection to be created.
	//    @return - unique pointer to the new connection.
	virtual UniquePtr<Connection> CreateConnection(UniquePtr<Socket> socket) = 0;

	// Starts network routine (specific for the inherited node type).
	//    @return - error if something failed.
	virtual Optional<Error> Run() = 0;

	// Closes all active connections and stops network routine.
	//    @return - error if failed.
	virtual Optional<Error> ShutDown();

	// Sends provided command through connection that is specified by provided address.
	//    @param command - unique pointer to the command to be sent.
	//    @param address - address of the destination. Ignore this parameter if
	//                     you want to send command to all available connections.
	//    @return - error if function failed.
	Optional<Error> SendCommand(UniquePtr<Command> command,
	                            const HostAddress& address = HostAddress());

	// Executes provided command.
	//    @param cmd - reference to the command to be executed.
	//    @param connection - reference to the connection containing the command.
	//    @return - error if function failed.
	virtual Optional<Error> ExecuteCommand(Command& cmd, Connection& connection) = 0;

	// Closes connection with provided address.
	//    @param address - address of the connection to close.
	void CloseConnection(const HostAddress& address);

	// Returns 'true' if host is active.
	bool IsActive();

	// signals, use them to monitor activity of the node,
	// you are free to connect any slot to these signals
	Signal<void(const HostAddress& address)> launched;
	Signal<void()> stopped;
	Signal<void(const Error& err)> command_failed;
	Signal<void(const HostAddress& address)> connection_closed;

protected:
	// active connections - one connection per one node
	SyncRW< Array< UniquePtr<Connection> > > connections;

	// address of the current node
	HostAddress address;

	// thread-safe activity status
	Synchronized<bool> active;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//