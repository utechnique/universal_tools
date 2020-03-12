//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_connection.h"
#include "net/ut_host.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
class ConnectionJob : public Job
{
public:
	ConnectionJob(Connection& in_connection,
	              Array< UniquePtr<Action> > in_actions) : connection(in_connection)
	                                                     , actions(Move(in_actions))
	{}

	void Execute()
	{
		for (size_t i = 0; i < actions.GetNum(); i++)
		{
			// exit if connection is in closing state
			if (!connection.IsActive() || exit_request.Read())
			{
				break;
			}

			// execute action
			Optional<Error> action_error = actions[i]->Execute(connection);
			if (action_error && action_error.Get().GetCode() != ut::error::connection_closed)
			{
				connection.ReportCommandFailure(action_error.Get());
				break;
			}
		}

		// close connection
		connection.ShutDown();
	}

private:
	Connection& connection;
	Array< UniquePtr<Action> > actions;
};


//----------------------------------------------------------------------------//
// Constructor
//    @param owner - host, owner of the connection
//    @param socket - socket of the established connection
//    @param actions - array of the actions to customize
//                     communication protocol
Connection::Connection(class Host& owner,
                       UniquePtr<Socket> in_socket,
                       Array< UniquePtr<Action> > in_actions) : host(owner)
                                                              , socket(Move(in_socket))
                                                              , active(true)
{
	// get host address from socket
	UniquePtr<Socket>& locked_socked = socket.Lock();
	Result<HostAddress, Error> getaddr_result = locked_socked->GetAddr();
	socket.Unlock();

	// validate address
	if (getaddr_result)
	{
		address = getaddr_result.GetResult();
	}
	else
	{
		throw Error(getaddr_result.GetAlt());
	}

	// create connection job
	job = MakeUnique<ConnectionJob>(*this, Move(in_actions));
}

// Destructor, shuts socket down and closes connection thread.
Connection::~Connection()
{
	// close thread and shut down socket,
	// note that there is no need to ask host for the deletion
	// of the current connection because it's already being
	// destructed (we are in the destructor)
	ShutDown(false);
}

// Starts current connection in a separate thread
Optional<Error> Connection::RunInSeparateThread()
{
	// validate thread job
	if (!job)
	{
		String description = "Thread job is empty. Note that RunInSeparateThread() ";
		description += "can be called only once after a connection was created.";
		return Error(error::fail, description);
	}

	// create new thread
	thread = MakeUnique<Thread>(Move(job));

	// success
	return Optional<Error>();
}

// Shuts connection down
//    @param delete_connection - this boolean indicates if connection
//                               needs to be deleted by host.
//    @return - error if function failed.
Optional<Error> Connection::ShutDown(bool delete_by_host)
{
	// lock activity status variable
	ScopeSyncLock<bool> lock(active);

	// disactivate connection
	if (lock.Get())
	{
		lock.Set(false);
	}
	else
	{
		// connection is already closed
		return Error(error::fail, "Connection is already closed.");
	}

	// unlock activity status variable
	lock.Unlock();

	// shut down socket to make 'graceful' disconnect
	const Socket& locked_socket = LockSocket();
	locked_socket.ShutDown();
	UnlockSocket();

	// close thread
	thread.Delete();

	// ask host to delete current connection
	if (delete_by_host)
	{
		host.CloseConnection(address);
	}

	// success
	return Optional<Error>();
}

// Returns a reference to the locked socket, you must call
// Connection::UnlockSocket() after work is done.
const Socket& Connection::LockSocket()
{
	return socket.Lock().GetRef();
}

// Unlocks connection socket.
void Connection::UnlockSocket()
{
	socket.Unlock();
}

// Adds a new command to the end of the stack. Provided
// command will be moved.
//    @param command - unique pointer to the command to be added.
//    @return - error if function failed.
Optional<Error> Connection::AddCommand(UniquePtr<Command> command)
{
	// lock commands
	ScopeSyncLock< Array< UniquePtr<Command> > > locked_commands(commands);
	Array< UniquePtr<Command> >& cmd_array = locked_commands.Get();

	// add command
	if (!cmd_array.Add(Move(command)))
	{
		return Error(error::out_of_memory);
	}

	// success
	return Optional<Error>();
}

// Adds a new command to the end of the stack. Provided
// command will be copied.
//    @param command - unique pointer to the command to be added.
//    @return - error if function failed.
Optional<Error> Connection::AddCommand(const Command& command)
{
	// create a copy
	const DynamicType& dynamic_type = command.Identify();
	Command* copy_ptr = static_cast<Command*>(dynamic_type.CloneObject(command));
	if (!copy_ptr)
	{
		return Error(error::not_implemented, "Command wasn't registered.");
	}

	// lock commands
	ScopeSyncLock< Array< UniquePtr<Command> > > locked_commands(commands);
	Array< UniquePtr<Command> >& cmd_array = locked_commands.Get();

	// add command
	UniquePtr<Command> copy(copy_ptr);
	if (!cmd_array.Add(Move(copy)))
	{
		return Error(error::out_of_memory);
	}

	// success
	return Optional<Error>();
}

// Withdraws one oldest command from the stack,
// or returns an error if stack is empty
Result<UniquePtr<Command>, Error> Connection::PickCommand()
{
	ScopeSyncLock< Array< UniquePtr<Command> > > locked_commands(commands);
	Array< UniquePtr<Command> >& cmd_array = locked_commands.Get();
	if (cmd_array.GetNum() == 0)
	{
		return MakeError(error::empty);
	}
	UniquePtr<Command> cmd(cmd_array.MoveElement(0));
	cmd_array.Remove(0);
	return Move(cmd);
}

// Executes provided command either on server side or on client side.
//    @param cmd - reference to the command to be executed.
//    @return - error if function failed.
Optional<Error> Connection::ExecuteCommand(Command& cmd)
{
	return host.ExecuteCommand(cmd, *this);
}

// Calls corresponding host signal (that some command failed)
void Connection::ReportCommandFailure(const Error& err) const
{
	host.command_failed(err);
}

// Returns 'true' if connection is still active
bool Connection::IsActive()
{
	return active.Get();
}

// Returns the address of the connection
HostAddress Connection::GetAddress() const
{
	return address;
}

// Returns a reference to the host node
Host& Connection::GetHost()
{
	return host;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//