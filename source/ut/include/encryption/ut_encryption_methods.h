//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "streams/ut_binary_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Enumeration of the encryption methods.
// Note that specialized ut::EncryptionStream<> template must be implemented
// for each of these methods.
namespace encryption
{
	enum Method
	{
		XOR,
		AES128
	};
}

//----------------------------------------------------------------------------//
// ut::EncryptionStreamBase is an abstract base class for encryption streams.
// These streams are binary streams that are able to encrypt and decrypt their
// data buffer. ut::EncryptionStreamBase forces inherited classes to implement
// Encrypt() and Decrypt() methods.
class EncryptionStreamBase : public BinaryStream
{
public:
	// Encrypts stream buffer using provided password.
	//    @param password - intermediate string to generate final encryption key,
	//                      this key will be generated using pbkdf2 function.
	//    @param pbkdf2_iterations - number of iterations for the pbkdf2 function
	//                               to generate encryption key from the provided
	//                               password.
	//    @return  - error if failed.
	virtual Optional<Error> Encrypt(const String& password,
	                                uint32 pbkdf2_iterations = 2048) = 0;

	// Decrypts stream buffer using provided password.
	//    @param password - intermediate string to generate final decryption key,
	//                      this key will be generated using pbkdf2 function.
	//    @param pbkdf2_iterations - number of iterations for the pbkdf2 function
	//                               to generate decryption key from the provided
	//                               password.
	//    @return  - error if failed.
	virtual Optional<Error> Decrypt(const String& password,
	                                uint32 pbkdf2_iterations = 2048) = 0;
};

//----------------------------------------------------------------------------//
// ut::EncryptionStream is a template class for encryption streams.
// Stream for the every encryption method must be declared separately using
// specialization of the EncryptionStream<> template.
template<encryption::Method EncryptionMethod> class EncryptionStream;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//