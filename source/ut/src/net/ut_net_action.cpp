//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_net_action.h"
#include "net/ut_connection.h"
#include "streams/ut_binary_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// Constructor
//    @param command_name - name of the registered command class name,
//                          ignore this parameter to receive all types of
//                          commands.
ReceiveCommand::ReceiveCommand(const String& command_name) : cmd_name(command_name)
{ }

// Receives serialized command, deserializes it and executes.
//    @param connection - connection executing current action.
//    @return - error if failed.
Optional<Error> ReceiveCommand::Execute(Connection& connection)
{
	// receive command
	Array<byte> data;
	const Socket& socket = connection.LockSocket();
	Result<int, Error> recv_result = socket.RecvArray<byte>(data);
	connection.UnlockSocket();
	if (!recv_result)
	{
		return recv_result.MoveAlt();
	}

	// create binary stream from the received data
	BinaryStream stream;
	stream.SetBuffer(Move(data));

	// deserialize command
	UniquePtr<Command> cmd;
	Optional<Error> deserialization_error = meta::Snapshot::Capture(cmd).Load(stream);
	if (deserialization_error)
	{
		return deserialization_error;
	}

	// validate command type, but only if command name was set
	if (cmd_name.Length() != 0)
	{
		const DynamicType& dynamic_type = cmd->Identify();
		if (dynamic_type.GetName() != cmd_name)
		{
			return Error(error::types_not_match);
		}
	}

	// execute command
	Optional<Error> execution_error = connection.ExecuteCommand(cmd.GetRef());
	if (execution_error)
	{
		connection.ReportCommandFailure(execution_error.Get());
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Constructor
//    @param command - unique pointer to the command to be sent.
SendCommand::SendCommand(UniquePtr<Command> command) : cmd(Move(command))
{ }

// Sends serialized command and calls ut::net::Command::sent signal.
//    @param connection - connection executing current action.
//    @return - error if failed.
Optional<Error> SendCommand::Execute(Connection& connection)
{
	// serialize command
	BinaryStream stream;
	Optional<Error> serialization_error = meta::Snapshot::Capture(cmd).Save(stream);

	// validate serialization result
	if (serialization_error)
	{
		return serialization_error;
	}

	// send stream data
	const Array<byte>& data = stream.GetBuffer();
	const Socket& socket = connection.LockSocket();
	Result<int, Error> send_result = socket.SendArray<byte>(data);
	connection.UnlockSocket();
	if (!send_result)
	{
		return send_result.MoveAlt();
	}

	// call slots of the 'sent' signal (and exit if returned error)
	Result<Optional<Error>, Error> signal_result = cmd->sent(connection);
	if (signal_result)
	{
		const Optional<Error>& signal_error = signal_result.Get();
		if (signal_error)
		{
			return signal_result.Move();
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Constructor
//    @param idle_cmd_ptr - unique pointer to the command to be sent in case
//                          if connection stack is empty.
SendCommandFromStack::SendCommandFromStack(UniquePtr<Command> idle_cmd_ptr) : idle_cmd(Move(idle_cmd_ptr))
{ }

// Picks command from connection stack, than serializes it and sends to the
// receiver, also calls ut::net::Command::sent signal.
//    @param connection - connection executing current action.
//    @return - error if failed.
Optional<Error> SendCommandFromStack::Execute(Connection& connection)
{
	// pick oldest
	Result<UniquePtr<Command>, Error> pick_result = connection.PickCommand();

	// move command if exist
	UniquePtr<Command> picked_cmd = pick_result ? pick_result.Move() : UniquePtr<Command>(nullptr);

	// serialize command
	BinaryStream stream;
	UniquePtr<Command>& cmd = pick_result ? picked_cmd : idle_cmd;
	Optional<Error> serialization_error = meta::Snapshot::Capture(cmd).Save(stream);

	// validate serialization result
	if (serialization_error)
	{
		return serialization_error;
	}

	// send stream data
	const Array<byte>& data = stream.GetBuffer();
	const Socket& socket = connection.LockSocket();
	Result<int, Error> send_result = socket.SendArray<byte>(data);
	connection.UnlockSocket();
	if (!send_result)
	{
		return send_result.MoveAlt();
	}

	// call slots of the 'sent' signal (and exit if returned error)
	Result<Optional<Error>, Error> signal_result = cmd->sent(connection);
	if (signal_result)
	{
		const Optional<Error>& signal_error = signal_result.Get();
		if (signal_error)
		{
			return signal_result.Move();
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
// Constructor.
//    @param looped_actions - array of actions to be looped.
Loop::Loop(Array< UniquePtr<Action> > looped_actions) : actions(Move(looped_actions))
{ }

// Executes sequentially every action from the @actions array in a loop.
//    @param connection - connection executing current action.
//    @return - error if failed.
Optional<Error> Loop::Execute(Connection& connection)
{
	while (connection.IsActive())
	{
		for (size_t i = 0; i < actions.GetNum(); i++)
		{
			Optional<Error> execution_error = actions[i]->Execute(connection);
			if (execution_error)
			{
				return execution_error;
			}
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//