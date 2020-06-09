//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "pointers/ut_unique_ptr.h"
#include "net/ut_socket.h"
#include "net/ut_net_cmd.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// ut::net::Action is a base class for network actions. 'Actions' are used to
// form user-defined communication protocol for network connections. You can
// create sequence of actions inside ut::net::Connection object to define
// it's behaviour.
class Action
{
public:
	// Virtual destructor.
	virtual ~Action() = default;

	// Executes an action. Must be implemented by child classes.
	//    @param connection - connection executing current action.
	//    @return - error if failed.
	virtual Optional<Error> Execute(class Connection& connection) = 0;
};

//----------------------------------------------------------------------------//
// ut::net::ReceiveCommand is a network action waiting for the specific (or any)
// command. This action receives a command, deserializes and executes it.
class ReceiveCommand : public Action
{
public:
	// Constructor
	//    @param command_name - name of the registered command class name,
	//                          ignore this parameter to receive all types of
	//                          commands.
	ReceiveCommand(const String& command_name = String());

	// Receives serialized command, deserializes it and executes.
	//    @param connection - connection executing current action.
	//    @return - error if failed.
	Optional<Error> Execute(class Connection& connection);

private:
	// name of the registered command class name, leave
	// this member empty to receive all types of commands.
	const String cmd_name;
};

//----------------------------------------------------------------------------//
// ut::net::SendCommand is a network action sending specific command. This
// action serializes provided command object and sends to the receiver.
// Also you can perform actions after sending a command using
// 'ut::net::Command::sent' signal of this command.
class SendCommand : public Action
{
public:
	// Constructor
	//    @param command - unique pointer to the command to be sent.
	SendCommand(UniquePtr<Command> command);

	// Sends serialized command and calls ut::net::Command::sent signal.
	//    @param connection - connection executing current action.
	//    @return - error if failed.
	Optional<Error> Execute(class Connection& connection);

private:
	// unique pointer to the command to be sent
	UniquePtr<Command> cmd;
};

//----------------------------------------------------------------------------//
// ut::net::SendCommandFromStack is the same as ut::net::SendCommand, but sends
// a first command from the stack of the owning connection instead of custom
// command. Also calls 'ut::net::Command::sent' signal of this picked command.
class SendCommandFromStack : public Action
{
public:
	// Constructor
	//    @param idle_cmd_ptr - unique pointer to the command to be sent in case
	//                          if connection stack is empty.
	SendCommandFromStack(UniquePtr<Command> idle_cmd_ptr);

	// Picks command from connection stack, than serializes it and sends to the
	// receiver, also calls ut::net::Command::sent signal.
	//    @param connection - connection executing current action.
	//    @return - error if failed.
	Optional<Error> Execute(class Connection& connection);

private:
	// unique pointer to the command to be sent
	// in case if connection stack is empty.
	UniquePtr<Command> idle_cmd;
};

//----------------------------------------------------------------------------//
// ut::net::Loop is a network action performing sequence of other network
// actions in a loop.
class Loop : public Action
{
public:
	// Constructor.
	//    @param looped_actions - array of actions to be looped.
	Loop(Array< UniquePtr<Action> > looped_actions);

	// Executes sequentially every action from the @actions array in a loop.
	//    @param connection - connection executing current action.
	//    @return - error if failed.
	Optional<Error> Execute(class Connection& connection);

private:
	// array of actions
	Array< UniquePtr<Action> > actions;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//