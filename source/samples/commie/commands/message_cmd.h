//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "../commie_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// commie::MessageCmd is a network command for sending/receiving text messages.
// Message can be encrypted, can be sent to one receiver or to the all clients.
class MessageCmd : public ut::net::Command
{
public:
	// Constructor
	//    @param msg - reference to the string containing text of a message.
	//    @param address - reference to the network address of the receiver.
	//    @param apply_encryption - boolean whether to encrypt message or not.
	//    @param password - encryption password, useless if @apply_encryption
	//                      is 'false'.
	MessageCmd(const ut::String& msg = ut::String(),
	           const ut::net::HostAddress& address = ut::net::HostAddress(),
	           bool apply_encryption = false,
	           const ut::String& password = ut::String());

	// Copy constructor
	MessageCmd(const MessageCmd& copy);

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

	// Helper function to decorate text of the message with tags:
	// [private], [encrypted] and client name.
	//    @param message - text of the message.
	//    @param is_encrypted - adds '[encrypted]' tag.
	//    @param is_private - adds '[private]' tag.
	//    @param sender_name - name of the client who sent the message.
	static ut::String ApplyTags(const ut::String& message,
	                            bool is_encrypted,
	                            bool is_private,
	                            const ut::String& sender_name);
private:
	// binary buffer containing text of the message, can be encrypted
	ut::Array<ut::byte> buffer;

	// address of the receiver
	ut::net::HostAddress dst_address;

	// name of a sender
	ut::String sender_name;

	// adds '[encrypted]' tag to the final message and
	// informs that message must be decrypted before using
	bool is_encrypted;

	// adds '[encrypted]' tag to the final message
	bool is_private;
};


//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//