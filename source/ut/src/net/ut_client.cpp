//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_client.h"
#include "net/ut_socket_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// Constructor
//    @param address - address of the server.
//    @param conn_interval_ms - connection attempt frequency
//                              in millisecons.
Client::Client(const HostAddress& address,
               uint32 conn_interval_ms) : Host(address)
                                        , connection_interval_ms(conn_interval_ms)
{ }

//----------------------------------------------------------------------------->
// Destructor
Client::~Client()
{ }

//----------------------------------------------------------------------------->
// Tries to connect to the server and establishes a connection if succeeded.
// If connection was lost, tries to connect again.
//    @return - error if failed.
Optional<Error> Client::Run()
{
	// activate host
	active.Set(true);
	launched(address);

	// make connection attempts
	while (active.Get())
	{
		ScopeSyncRWLock< Array< UniquePtr<Connection> > > connection_lock(connections, access_write);

		// connect only if there is no established connection
		if (connection_lock.Get().GetNum() == 0)
		{
			// create new socket
			UniquePtr<Socket> socket(new Socket(address.ip, address.port));

			// try to connect
			ut::Optional<ut::Error> connect_error = socket->Connect();
			if (connect_error)
			{
				// failure
				connection_failed(address);
			}
			else
			{
				// create new connection
				UniquePtr<Connection> connection = CreateConnection(Move(socket));
				if (!connection_lock.Get().Add(Move(connection)))
				{
					return Error(error::out_of_memory);
					connection_failed(address);
				}

				// signalize that connected successfully
				connected_to_server(address);

				// start connection in a separate thread
				connection_lock.Get().GetLast()->RunInSeparateThread();
			}
		}

		// unlock the scope before sleep
		connection_lock.Unlock();

		// wait before the next attempt
		Sleep(connection_interval_ms);
	}

	// signalize that host stopped working
	stopped();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Executes provided command.
//    @param cmd - reference to the command to be executed.
//    @param connection - reference to the connection containing the command.
//    @return - error if function failed.
Optional<Error> Client::ExecuteCommand(Command& cmd, Connection& connection)
{
	return cmd.ExecuteOnClient(connection);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//