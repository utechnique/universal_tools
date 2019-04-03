//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "encryption/ut_xor.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Encrypts data with XOR
//    @param key - key to encrypt with.
//    @param key_size - length of the key in bytes.
//    @param data - data to encrypt.
//    @param data_size - size of the data in bytes.
void EncryptXor(byte* data, size_t data_size, const byte* key, size_t key_size)
{
	for (size_t i = 0; i < data_size; i++)
	{
		data[i] = data[i] ^ key[i % key_size];
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
