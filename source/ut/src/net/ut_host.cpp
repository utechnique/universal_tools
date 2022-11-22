//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_host.h"
#include "net/ut_socket_system.h"
#include "dbg/ut_log.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// Constructor
//    @param in_address - network address of the node.
Host::Host(const HostAddress& in_address) : address(in_address)
                                          , active(false)
{ }

//----------------------------------------------------------------------------->
// Destructor
Host::~Host()
{ }

//----------------------------------------------------------------------------->
// Closes all active connections and stops network routine.
//    @return - error if failed.
Optional<Error> Host::ShutDown()
{
	// disactivate host
	active = false;

	// close all connections
	ScopeSyncRWLock< Array< UniquePtr<Connection> > > locked_connections(connections, access_write);
	locked_connections.Get().Reset();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Sends provided command through connection that is specified by provided address.
//    @param command - unique pointer to the command to be sent.
//    @param address - address of the destination. Ignore this parameter if
//                     you want to send command to all available connections.
//    @return - error if function failed.
Optional<Error> Host::SendCommand(UniquePtr<Command> command,
                                  const HostAddress& address)
{
	// lock connections
	ScopeSyncRWLock< Array< UniquePtr<Connection> > > connections_scope_lock(connections, access_read);
	Array< UniquePtr<Connection> >& locked_connections = connections_scope_lock.Get();

	// add command
	if (address.IsValid())
	{
		// if address is valid - just find the connection and move provided command
		for (size_t i = 0; i < locked_connections.Count(); i++)
		{
			if (address == locked_connections[i]->GetAddress())
			{
				Optional<Error> add_error = locked_connections[i]->AddCommand(Move(command));
				if (add_error)
				{
					return add_error;
				}
				break;
			}
		}
	}
	else
	{
		// if address is invalid - add a copy of the command to all connections
		for (size_t i = 0; i < locked_connections.Count(); i++)
		{
			const Command& cref = command.GetRef();
			Optional<Error> add_error = locked_connections[i]->AddCommand(cref);
			if (add_error)
			{
				return add_error;
			}
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Closes connection with provided address.
//    @param address - address of the connection to close.
void Host::CloseConnection(const HostAddress& address)
{
	// lock connections
	ScopeSyncRWLock< Array< UniquePtr<Connection> > > connections_scope_lock(connections, access_write);
	Array< UniquePtr<Connection> >& locked_connections = connections_scope_lock.Get();

	// find and delete connection with matching address
	for (size_t i = 0; i < locked_connections.Count(); i++)
	{
		if (locked_connections[i]->GetAddress() == address)
		{
			// we must create new address object, because provided argument
			// can be destructed with the connection, that owns this reference
			// (if this call was performed from Connection::ShutDown())
			HostAddress address_copy(address);

			// delete desired connection
			locked_connections.Remove(i);

			// signalize that connection was closed
			connection_closed(address_copy);

			// break the loop
			break;
		}
	}
}

//----------------------------------------------------------------------------->
// Returns 'true' if host is active.
bool Host::IsActive()
{
	return active.Get();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//