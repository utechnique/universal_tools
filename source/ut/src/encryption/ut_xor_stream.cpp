//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "encryption/ut_xor_stream.h"
#include "encryption/ut_pbkdf2.h"
#include "encryption/ut_xor.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Encrypts stream buffer using provided password.
//    @param password - intermediate string to generate final encryption key,
//                      this key will be generated using pbkdf2 function.
//    @param pbkdf2_iterations - number of iterations for the pbkdf2 function
//                               to generate encryption key from the provided
//                               password.
//    @return  - error if failed.
Optional<Error> EncryptionStream<encryption::XOR>::Encrypt(const String& password,
	                                                       uint32 pbkdf2_iterations)
{
	// generate 16-bit key from the password
	byte key[16];
	Pbkdf2Sha256Function pbkdf2_sha256;
	pbkdf2_sha256.Calculate(reinterpret_cast<const byte*>(password.ToCStr()),
		                    static_cast<uint32>(password.Length()),
		                    reinterpret_cast<const byte*>(password.ToCStr()),
		                    static_cast<uint32>(password.Length()),
		                    pbkdf2_iterations,
		                    key,
		                    16);

	// encrypt data
	EncryptXor(data.GetAddress(), data.GetNum(), key, 16);

	// success
	return Optional<Error>();
}

// Decrypts stream buffer using provided password.
//    @param password - intermediate string to generate final decryption key,
//                      this key will be generated using pbkdf2 function.
//    @param pbkdf2_iterations - number of iterations for the pbkdf2 function
//                               to generate decryption key from the provided
//                               password.
//    @return  - error if failed.
Optional<Error> EncryptionStream<encryption::XOR>::Decrypt(const String& password,
	                                                       uint32 pbkdf2_iterations)
{
	// generate 16-bit key from the password
	byte key[16];
	Pbkdf2Sha256Function pbkdf2_sha256;
	pbkdf2_sha256.Calculate(reinterpret_cast<const byte*>(password.ToCStr()),
		                    static_cast<uint32>(password.Length()),
		                    reinterpret_cast<const byte*>(password.ToCStr()),
		                    static_cast<uint32>(password.Length()),
		                    pbkdf2_iterations,
		                    key,
		                    16);

	// decrypt data
	EncryptXor(data.GetAddress(), data.GetNum(), key, 16);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
