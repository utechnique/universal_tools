//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "pointers/ut_unique_ptr.h"
#include "system/ut_thread.h"
#include "net/ut_host_address.h"
#include "net/ut_socket.h"
#include "net/ut_net_action.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//

class Connection : public NonCopyable
{
	friend class ConnectionJob;
public:
	// Constructor
	//    @param host - host, owner of the connection
	//    @param socket - socket of the established connection
	//    @param actions - array of the actions to customize
	//                     communication protocol
	Connection(class Host& host,
	           RValRef< UniquePtr<Socket> >::Type socket,
	           RValRef< Array< UniquePtr<Action> > >::Type actions);

	// Destructor, shuts socket down and closes connection thread.
	~Connection();

	// Starts current connection in a separate thread
	Optional<Error> RunInSeparateThread();

	// Returns a reference to the locked socket, you must call
	// Connection::UnlockSocket() after work is done.
	const Socket& LockSocket();

	// Unlocks connection socket.
	void UnlockSocket();

	// Adds a new command to the end of the stack. Provided
	// command will be moved.
	//    @param command - unique pointer to the command to be added.
	//    @return - error if function failed.
	Optional<Error> AddCommand(RValRef< UniquePtr<Command> >::Type command);

	// Adds a new command to the end of the stack. Provided
	// command will be copied.
	//    @param command - unique pointer to the command to be added.
	//    @return - error if function failed.
	Optional<Error> AddCommand(const Command& command);

	// Withdraws one oldest command from the stack,
	// or returns an error if stack is empty
	Result<UniquePtr<Command>, Error> PickCommand();

	// Executes provided command either on server side or on client side.
	//    @param cmd - reference to the command to be executed.
	//    @return - error if function failed.
	Optional<Error> ExecuteCommand(Command& cmd);

	// Calls corresponding host signal (that some command failed)
	void ReportCommandFailure(const Error& err) const;

	// Returns 'true' if connection is still active
	bool IsActive();

	// Returns the address of the connection
	HostAddress GetAddress() const;

	// Returns a reference to the host node
	class Host& GetHost();

private:
	// Shuts connection down.
	//    @param delete_by_host - this boolean indicates if connection
	//                            needs to be deleted by host.
	//    @return - error if function failed.
	Optional<Error> ShutDown(bool delete_by_host = true);

	// socket of the corresponding host
	Synchronized< UniquePtr<Socket> > socket;
	
	// address of the host
	HostAddress address;

	// commands awaiting for being sent
	Synchronized< Array< UniquePtr<Command> > > commands;

	// indicates if connection is still active
	Synchronized<bool> active;

	// every connection runs a separate thread
	UniquePtr<Job> job;
	UniquePtr<Thread> thread;

	// owner of the connection
	class Host& host;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//