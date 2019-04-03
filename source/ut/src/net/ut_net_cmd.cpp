//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "net/ut_net_cmd.h"
#include "net/ut_connection.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// This combiner will return ut::Error if at least one slot returnes error.
Optional<Error> CmdSignalCombiner::operator()(const Optional<Error>& err)
{
	if (!current_state)
	{
		current_state = err;
	}
	return current_state;
}

//----------------------------------------------------------------------------//
// Executes command if received by client.
//    @param connection - connection owning the command.
//    @return - error if failed.
Optional<Error> Command::ExecuteOnClient(Connection& connection)
{
	return Optional<Error>();
}

// Executes command if received by server.
//    @param connection - connection owning the command.
//    @return - error if failed.
Optional<Error> Command::ExecuteOnServer(Connection& connection)
{
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//