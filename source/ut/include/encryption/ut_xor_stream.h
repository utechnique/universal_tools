//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "encryption/ut_encryption_methods.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::EncryptionStream<encryption::XOR> is a specialized EncryptionStream<>
// template for the XOR method of encryption.
template<>
class EncryptionStream<encryption::XOR> : public EncryptionStreamBase
{
public:
	// Encrypts stream buffer using provided password.
	//    @param password - intermediate string to generate final encryption key,
	//                      this key will be generated using pbkdf2 function.
	//    @param pbkdf2_iterations - number of iterations for the pbkdf2 function
	//                               to generate encryption key from the provided
	//                               password.
	//    @return  - error if failed.
	Optional<Error> Encrypt(const String& password,
	                        uint32 pbkdf2_iterations = 2048);

	// Decrypts stream buffer using provided password.
	//    @param password - intermediate string to generate final decryption key,
	//                      this key will be generated using pbkdf2 function.
	//    @param pbkdf2_iterations - number of iterations for the pbkdf2 function
	//                               to generate decryption key from the provided
	//                               password.
	//    @return  - error if failed.
	Optional<Error> Decrypt(const String& password,
	                        uint32 pbkdf2_iterations = 2048);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//