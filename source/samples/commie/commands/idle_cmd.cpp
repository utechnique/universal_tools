//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "idle_cmd.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ut::net::Command, commie::IdleCmd, "idle")
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Default time amount to sleep in milliseconds.
const ut::uint32 IdleCmd::skDefaultFrequency = 1000;

//----------------------------------------------------------------------------//
// Constructor
//    @param ms_to_wait - time amount to sleep in milliseconds.
IdleCmd::IdleCmd(ut::uint32 ms_to_wait) : milliseconds_to_wait(ms_to_wait)
{ }

// Copy constructor
IdleCmd::IdleCmd(const IdleCmd& copy) : milliseconds_to_wait(copy.milliseconds_to_wait)
{ }

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& IdleCmd::Identify() const
{
	return ut::Identify(this);
}

// Serialization
void IdleCmd::Serialize(ut::MetaStream& stream)
{
	stream << milliseconds_to_wait;
}

// Executes command if received by client.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> IdleCmd::ExecuteOnClient(ut::net::Connection& connection)
{
	// wait
	ut::Sleep(milliseconds_to_wait);

	// success
	return ut::Optional<ut::Error>();
}

// Executes command if received by server.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> IdleCmd::ExecuteOnServer(ut::net::Connection& connection)
{
	// nothing to do

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//