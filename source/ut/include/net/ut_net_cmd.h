//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_stream.h"
#include "meta/ut_polymorphic.h"
#include "meta/ut_serializable.h"
#include "events/ut_signal.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(net)
//----------------------------------------------------------------------------//
// This combiner will return ut::Error if at least one slot returnes error.
struct CmdSignalCombiner
{
	Optional<Error> operator()(const Optional<Error>& err);
	Optional<Error> current_state;
};

//----------------------------------------------------------------------------//
// ut::net::Command is a base class for network commands. 'Command' is the
// smallest portion of meaningful information in ut network library. Command
// can be sent to another node and be executed there. Node must know what
// command to expect - so you need to define communication protocol between nodes.
// Use ut::net::Host::CreateConnection() to create connection with custom
// sequence of actions before sending commands. Also you need to register every
// command type manually or using UT_REGISTER_TYPE() macro.
class Command : public Serializable, public Polymorphic
{
public:
	// Serializes members of the command.
	//    @param stream - meta stream to serialize members in.
	virtual void Serialize(MetaStream& stream) = 0;

	// Identify() method must be implemented for the polymorphic types.
	virtual const DynamicType& Identify() const = 0;

	// Executes command if received by client.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	virtual Optional<Error> ExecuteOnClient(class Connection& connection);

	// Executes command if received by server.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	virtual Optional<Error> ExecuteOnServer(class Connection& connection);

	// Use this signal to perform actions after the command was sent.
	Signal<Optional<Error>(class Connection&), CmdSignalCombiner> sent;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(net)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//