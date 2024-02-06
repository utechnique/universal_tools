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
// ut::Sha256Function is class for generating 256-bit hash from custom data
// and cipher key using hmac-sha256 algorithm.
class HmacSha256Function
{
public:
	// Generates 256-bit hash from provided data.
	//    @param key - pointer to the first byte of the key.
	//    @param key_len - size of the provided key in bytes.
	//    @param msg - pointer to the first byte of the data.
	//    @param msg_len - size of the provided data in bytes.
	//    @param digest - pointer to the first byte of destination
	//                    buffer (32 bytes) for the hash.
	void Calculate(const byte* key,
	               uint32 key_len,
	               const byte* msg,
	               uint32 msg_len,
	               byte* digest);

	// Size of the final hash in bytes.
	static const uint32 skDigestSize;

private:
	// Size of the operating block of the sha256 function in bytes.
	static const uint32 skBlockSize;

	// Block-sized inner padding, consisting of repeated bytes valued 0x36
	static const byte skIpad[64];

	// Block-sized outer padding, consisting of repeated bytes valued 0x5c
	static const byte skOpad[64];
};

//----------------------------------------------------------------------------//
// Text variant of hmac-sha-256 function.
//    @param message - string to generate hash from.
//    @param key - key to sign hash.
//    @return - 256-bit hash in text form.
String HmacSha256(const String& key, const String& message);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//