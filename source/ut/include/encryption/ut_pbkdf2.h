//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Pbkdf2Sha256Function is class for generating custom-length hash from
// custom data using pbkdf2-sha256 algorithm.
class Pbkdf2Sha256Function
{
public:
	// Generates custom-length hash from provided data.
	//    @param password - pointer to the first byte of the password.
	//    @param password_len - size of the provided password in bytes.
	//    @param salt - pointer to the first byte of the salt (ssid).
	//    @param salt_len - size of the provided salt in bytes.
	//    @param iterations - number of iterations.
	//    @param buffer - pointer to the first byte of the destination buffer.
	//    @param dk_len - size of the destination buffer in bytes.
	void Calculate(const byte* password,
	               uint32 password_len,
	               const byte* salt,
	               uint32 salt_len,
	               uint32 iterations,
	               byte* buffer,
	               uint32 dk_len);

private:
	// Size of the sha-256 hash function result in bytes.
	static const uint32 skHashLength;
};

//----------------------------------------------------------------------------//
// Text variant of pbkdf2-sha-256 function.
//    @param password - string to generate hash from.
//    @param salt - salt (ssid).
//    @param iterations - number of iterations.
//    @param length - size of the resulting hash in bytes.
//    @return - string with the hash.
String Pbkdf2Sha256(const String& password,
                    const String& salt,
                    uint32 iterations,
                    uint32 length);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//