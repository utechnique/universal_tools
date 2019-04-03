//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::IdleCmd is a network command for idle loop of the connection, used
// when there are no other network commands in a stack.
class IdleCmd : public ut::net::Command
{
public:
	// Constructor
	//    @param ms_to_wait - time amount to sleep in milliseconds.
	IdleCmd(ut::uint32 ms_to_wait = 0);

	// Copy constructor
	IdleCmd(const IdleCmd& copy);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Serialization
	void Serialize(ut::MetaStream& stream);

	// Executes command if received by client.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	ut::Optional<ut::Error> ExecuteOnClient(ut::net::Connection& connection);

	// Executes command if received by server.
	//    @param connection - connection owning the command.
	//    @return - error if failed.
	ut::Optional<ut::Error> ExecuteOnServer(ut::net::Connection& connection);

	// Default time amount to sleep in milliseconds.
	static const ut::uint32 skDefaultFrequency;

private:
	// Time amount to sleep in milliseconds.
	ut::uint32 milliseconds_to_wait;
};


//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//