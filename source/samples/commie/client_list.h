//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "commie_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::ClientInfo is the smallest information that is needed to describe
// a client. Object of this class can be serialized.
class ClientInfo : public ut::meta::Reflective
{
public:
	// Serializes data.
	void Reflect(ut::meta::Snapshot& snapshot);

	// name of the client
	ut::String name;

	// network address of the client
	ut::net::HostAddress address;
};

// Container for descriptors of client connection.
typedef ut::Array<ClientInfo> ClientList;

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//