//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "../client.h"
#include "../connection.h"
#include "message_cmd.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ut::net::Command, commie::MessageCmd, "message")
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
//----------------------------------------------------------------------------//
// Constructor
//    @param msg - reference to the string containing text of a message.
//    @param address - reference to the network address of the receiver.
//    @param apply_encryption - boolean whether to encrypt message or not.
//    @param password - encryption password, useless if @apply_encryption
//                      is 'false'.
MessageCmd::MessageCmd(const ut::String& msg,
                       const ut::net::HostAddress& address,
                       bool apply_encryption,
                       const ut::String& password) : dst_address(address)
                                                   , is_encrypted(apply_encryption)
                                                   , is_private(false)
{
	if (apply_encryption)
	{
		// encrypt buffer
		ut::EncryptionStream<ut::encryption::AES128> encryption_stream;
		encryption_stream.Write(msg.GetAddress(), 1, msg.GetNum());
		encryption_stream.Encrypt(password);
		buffer = encryption_stream.GetBuffer();
	}
	else
	{
		// just copy message string to the buffer
		buffer.Resize(msg.GetNum());
		ut::memory::Copy(buffer.GetAddress(), msg.GetAddress(), msg.GetNum());
	}
}

// Copy constructor
MessageCmd::MessageCmd(const MessageCmd& copy) : buffer(copy.buffer)
                                               , dst_address(copy.dst_address)
                                               , is_encrypted(copy.is_encrypted)
                                               , is_private(copy.is_private)
                                               , sender_name(copy.sender_name)
{ }

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& MessageCmd::Identify() const
{
	return ut::Identify(this);
}

// Serialization
void MessageCmd::Serialize(ut::MetaStream& stream)
{
	stream << buffer;
	stream << dst_address.ip;
	stream << dst_address.port;
	stream << is_encrypted;
	stream << is_private;
	stream << sender_name;
}

// Executes command if received by client.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> MessageCmd::ExecuteOnClient(ut::net::Connection& connection)
{
	// get client host object
	commie::Client& host = static_cast<commie::Client&>(connection.GetHost());

	// get buffer of the encrypted message
	ut::Array<ut::byte> decrypted_buffer;
	if (is_encrypted)
	{
		ut::EncryptionStream<ut::encryption::AES128> encryption_stream;
		encryption_stream.SetBuffer(buffer);
		encryption_stream.Decrypt(host.GetEncryptionKey());
		decrypted_buffer = encryption_stream.GetBuffer();
	}
	else
	{
		decrypted_buffer = buffer;
	}

	// validate decrypted buffer
	ut::String decrypted_message("[invalid message]");
	for (size_t i = 0; i < buffer.GetNum(); i++)
	{
		// check that buffer has null-terminator
		char symbol = *reinterpret_cast<const char*>(decrypted_buffer.GetAddress() + i);
		if (symbol == '\0')
		{
			decrypted_message = reinterpret_cast<const char*>(decrypted_buffer.GetAddress());
			break;
		}
	}

	// message with applied tags
	ut::String modified_message = ApplyTags(decrypted_message, is_encrypted, is_private, sender_name);

	// display message in user interface
	UI& ui = host.GetUI();
	ui.DisplayText(modified_message);

	// success
	return ut::Optional<ut::Error>();
}

// Executes command if received by server.
//    @param connection - connection owning the command.
//    @return - error if failed.
ut::Optional<ut::Error> MessageCmd::ExecuteOnServer(ut::net::Connection& connection)
{
	// body of the new (modified) message
	ut::String modified_message;

	// add [private] tag
	if (dst_address.IsValid())
	{
		is_private = true;
	}

	// get the name of the sender and modify message
	commie::Connection& commie_connection = static_cast<commie::Connection&>(connection);
	sender_name = commie_connection.GetClientName();

	// send to the destination
	ut::UniquePtr<ut::net::Command> copy(new MessageCmd(*this));
	return connection.GetHost().SendCommand(Move(copy), dst_address);
}

// Helper function to decorate text of the message with tags:
// [pricate], [encrypted] and client name.
//    @param message - text of the message.
//    @param is_encrypted - adds '[encrypted]' tag.
//    @param is_private - adds '[pricate]' tag.
//    @param sender_name - name of the client who sent the message.
ut::String MessageCmd::ApplyTags(const ut::String& message,
                                 bool is_encrypted,
                                 bool is_private,
                                 const ut::String& sender_name)
{
	// message with applied tags
	ut::String modified_message;

	// add 'private' tag
	if (is_private)
	{
		modified_message += "[private]";
	}

	// add 'encrypted' tag
	if (is_encrypted)
	{
		modified_message += "[encrypted]";
	}

	// add space after tags
	if (is_private || is_encrypted)
	{
		modified_message += " ";
	}

	// add sender name
	modified_message += sender_name + ": ";

	// add message body
	modified_message += message + ut::cret;

	// exit
	return modified_message;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//