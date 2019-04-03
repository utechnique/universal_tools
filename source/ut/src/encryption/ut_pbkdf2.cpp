//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "encryption/ut_pbkdf2.h"
#include "encryption/ut_hmac.h"
#include "encryption/ut_xor.h"
#include "containers/ut_array.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Size of the sha-256 hash function result in bytes.
const uint32 Pbkdf2Sha256Function::skHashLength = 32;

//----------------------------------------------------------------------------//
// Generates custom-length hash from provided data.
//    @param password - pointer to the first byte of the password.
//    @param password_len - size of the provided password in bytes.
//    @param salt - pointer to the first byte of the salt (ssid).
//    @param salt_len - size of the provided salt in bytes.
//    @param iterations - number of iterations.
//    @param buffer - pointer to the first byte of the destination buffer.
//    @param dk_len - size of the destination buffer in bytes.
void Pbkdf2Sha256Function::Calculate(const byte* password,
                                     uint32 password_len,
                                     const byte* salt,
                                     uint32 salt_len,
                                     uint32 iterations,
                                     byte* buffer,
                                     uint32 dk_len)
{
	// initialize hmac function
	HmacSha256Function hmac_sha256;

	// calculate number of blocks
	uint32 blocks_num = dk_len / skHashLength;
	uint32 blocks_mod = dk_len % skHashLength;
	if (blocks_mod > 0)
	{
		blocks_num++;
	}

	// iterate all blocks
	for (uint32 i = 0; i < blocks_num; i++)
	{
		// current block
		byte T[skHashLength];

		// inner U block from previous iteration
		byte prev_U[skHashLength];

		// repeat hash function as much as it needed
		for (uint32 c = 0; c < iterations; c++)
		{
			// current U block
			byte U[skHashLength];

			// first block is special - it's a concatenation of salt and number of blocks
			if (c == 0)
			{
				int32 integer_n = static_cast<int32>(blocks_num);
				Array<byte> salt_plus_n(salt_len + sizeof(int32));
				memory::Copy(salt_plus_n.GetAddress(), salt, salt_len);
				memory::Copy(salt_plus_n.GetAddress() + salt_len, &integer_n, sizeof(int32));
				hmac_sha256.Calculate(password,
				                      password_len,
				                      salt_plus_n.GetAddress(),
				                      static_cast<uint32>(salt_plus_n.GetNum()),
				                      T);
				memory::Copy(prev_U, T, skHashLength); // save U block for the next iteration
			}
			else
			{
				hmac_sha256.Calculate(password, password_len, prev_U, skHashLength, U);
				EncryptXor(T, skHashLength, U, skHashLength);
				memory::Copy(prev_U, U, skHashLength); // save U block for the next iteration
			}
		}

		// concatenate to the destination buffer
		uint32 block_size = skHashLength;
		if (blocks_mod > 0 && i == blocks_num - 1)
		{
			block_size = blocks_mod;
		}
		memory::Copy(buffer + i*skHashLength, T, block_size);
	}
}

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
                    uint32 length)
{
	// buffer for the binary pbkdf2 result
	Array<byte> buffer(length);

	// calculate hash
	Pbkdf2Sha256Function pbkdf2_sha256;
	pbkdf2_sha256.Calculate(reinterpret_cast<const byte*>(password.GetAddress()),
	                        static_cast<uint32>(password.Length()),
	                        reinterpret_cast<const byte*>(salt.GetAddress()),
	                        static_cast<uint32>(salt.Length()),
	                        iterations,
	                        buffer.GetAddress(),
	                        length);

	// allocate memory for the output string
	String out(length * 2);

	// convert to text form
	for (uint32 i = 0; i < length; i++)
	{
		String digest_hex;
		digest_hex.Print("%02x", buffer[i]);
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
