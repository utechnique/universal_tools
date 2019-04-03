//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "encryption/ut_encryption_methods.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::EncryptionStream<encryption::AES128> is a specialized EncryptionStream<>
// template for the AES method of encryption.
template<>
class EncryptionStream<encryption::AES128> : public EncryptionStreamBase
{
public:
	// Constructor
	//    @param initialization_vector - 16-byte array to perform AES128
	//                                   encryption. Ignore this parameter to use
	//                                   default initialization vector.
	EncryptionStream(const byte initialization_vector[16] = skIv);

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

private:
	// AES128 initialization vector.
	byte iv[16];

	// Default initialization vector for AES128 encryption.
	// Represents 0x01, 0x02, 0x03 ... 0x0f sequence.
	static const byte skIv[16];
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//