//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "encryption/ut_xor.h"
#include "hash/ut_hmac.h"
#include "hash/ut_sha2.h"
#include "containers/ut_array.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Size of the operating block of the sha256 function in bytes.
const uint32 HmacSha256Function::skBlockSize = 64;

// Size of the final hash in bytes.
const uint32 HmacSha256Function::skDigestSize = 32;

// Block-sized inner padding, consisting of repeated bytes valued 0x36
const byte HmacSha256Function::skIpad[64] =
{
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36
};

// Block-sized outer padding, consisting of repeated bytes valued 0x5c
const byte HmacSha256Function::skOpad[64] =
{
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C,
	0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C, 0x5C
};

// Generates 256-bit hash from provided data.
//    @param key - pointer to the first byte of the key.
//    @param key_len - size of the provided key in bytes.
//    @param msg - pointer to the first byte of the data.
//    @param msg_len - size of the provided data in bytes.
//    @param digest - pointer to the first byte of destination
//                    buffer (32 bytes) for the hash.
void HmacSha256Function::Calculate(const byte* key,
                                   uint32 key_len,
                                   const byte* msg,
                                   uint32 msg_len,
                                   byte* digest)
{
	// block-sized key derived from the secret key, K;
	// either by padding to the right with 0s up to the block size,
	// or by hashing down to less than the block size first and then
	// padding to the right with zeros
	byte k0[64];

	// inner padded key
	byte si[64];

	// outer padded key
	byte so[64];

	// result of the inner hash function
	byte inner_hash[32];

	// parameter for the outer hash function
	byte outer_message[96];

	// sha256 function
	Sha256Function sha256;

	// generate block-sized key derived from the secret key
	if (key_len == skBlockSize)
	{
		memory::Copy(k0, key, skBlockSize);
	}
	else if (key_len < skBlockSize)
	{
		memory::Set(k0 + key_len, 0, skBlockSize - key_len);
		memory::Copy(k0, key, key_len);
	}
	else
	{
		sha256.Calculate(key, key_len, k0);
		memory::Set(k0 + skDigestSize, 0, skBlockSize - skDigestSize);
	}

	// calculate inner padded key using xor
	memory::Copy(si, k0, skBlockSize);
	EncryptXor(si, skBlockSize, skIpad, skBlockSize);

	// calculate outer padded key using xor
	memory::Copy(so, k0, skBlockSize);
	EncryptXor(so, skBlockSize, skOpad, skBlockSize);

	// calculate inner hash
	Array<byte> inner_data(skBlockSize + msg_len);
	memory::Copy(inner_data.GetAddress(), si, skBlockSize);
	memory::Copy(inner_data.GetAddress() + skBlockSize, msg, msg_len);
	sha256.Calculate(inner_data.GetAddress(),
	                 static_cast<uint32>(inner_data.Count()),
	                 inner_hash);

	// calculate outer hash
	memory::Copy(outer_message, so, skBlockSize);
	memory::Copy(outer_message + skBlockSize, inner_hash, skDigestSize);
	sha256.Calculate(outer_message, skBlockSize + skDigestSize, digest);
}

//----------------------------------------------------------------------------//
// Text variant of hmac-sha-256 function.
//    @param message - string to generate hash from.
//    @param key - key to sign hash.
//    @return - 256-bit hash in text form.
String HmacSha256(const String& key, const String& message)
{
	// Calculate hmac sha256 hash
	byte digest[HmacSha256Function::skDigestSize];
	HmacSha256Function hmac_sha256;
	hmac_sha256.Calculate(reinterpret_cast<const byte*>(key.GetAddress()),
	                      static_cast<uint32>(key.Length()),
	                      reinterpret_cast<const byte*>(message.GetAddress()),
	                      static_cast<uint32>(message.Length()),
	                      digest);

	// allocate memory for string
	String out(2 * HmacSha256Function::skDigestSize);

	// convert to text form
	for (int i = 0; i < HmacSha256Function::skDigestSize; i++)
	{
		String digest_hex;
		digest_hex.Print("%02x", digest[i]);
		memory::Copy(out.GetAddress() + i * 2, digest_hex.GetAddress(), 2);
	}

	// success
	return out;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
