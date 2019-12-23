//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_server.h"
#include "net/ut_socket_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// Constructor.
//    @param address - address of the clients that can be accepted, leave
//                     it empty to allow any connection.
//    @param in_accept_timeout_ms - server gives up listening after this
//                                  interval of time (in milliseconds),
//                                  then tries again in a loop.
Server::Server(const HostAddress& address,
               uint in_accept_timeout_ms) : Host(address)
                                          , accept_timeout_ms(in_accept_timeout_ms)
{ }

// Destructor.
Server::~Server()
{ }

// Listens for the new clients. If client was accepted - creates new connection.
//    @return - error if failed.
Optional<Error> Server::Run()
{
	// create new socket
	UniquePtr<Socket> socket(new Socket(address.ip, address.port));
	
	// bind address
	Optional<Error> bind_error = socket->Bind();
	if (bind_error)
	{
		return Error(bind_error.Get().GetCode(), "Failed to bind server socket.");
	}
	
	// start listening clients
	Optional<ut::Error> listen_error = socket->Listen();
	if (listen_error)
	{
		return Error(listen_error.Get().GetCode(), "Server socket failed to listen connections.");
	}

	// activate host
	active.Set(true);
	launched(address);

	// accept clients
	while (active.Get())
	{
		// use poll to set timeout for waiting a client
		Optional<ut::Error> poll_error = socket->Poll(accept_timeout_ms);
		if (poll_error)
		{
			continue;
		}
		else
		{
			Result<UniquePtr<Socket>, Error> accept_result = socket->Accept();
			if (accept_result)
			{
				// signalize that new client has been accepted
				Result<HostAddress, Error> get_addr_result = accept_result.GetResult()->GetAddr();
				if (get_addr_result)
				{
					client_accepted(get_addr_result.GetResult());
				}
				
				// create new connection
				UniquePtr<Connection> connection = CreateConnection(accept_result.MoveResult());

				// add connection to the array
				Array< UniquePtr<Connection> >& locked_connections = connections.Lock();
				bool add_connection_result = locked_connections.Add(Move(connection));
				connections.Unlock();

				// validate the result
				if (!add_connection_result)
				{
					return Error(error::out_of_memory);
				}

				// start connection in a separate thread
				locked_connections.GetLast()->RunInSeparateThread();
			}
			else
			{
				continue;
			}
		}
	}
	
	// signalize that host stopped working
	stopped();

	// success
	return Optional<Error>();
}

// Executes provided command.
//    @param cmd - reference to the command to be executed.
//    @param connection - reference to the connection containing the command.
//    @return - error if function failed.
Optional<Error> Server::ExecuteCommand(Command& cmd, Connection& connection)
{
	return cmd.ExecuteOnServer(connection);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//