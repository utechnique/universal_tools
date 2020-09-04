//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "encryption/ut_aes_stream.h"
#include "encryption/ut_pbkdf2.h"
#include "encryption/ut_aes.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Constructor
//    @param initialization_vector - 16-byte array to perform AES128
//                                   encryption. Ignore this parameter to use
//                                   default initialization vector.
EncryptionStream<encryption::AES128>::EncryptionStream(const byte initialization_vector[16])
{
	memory::Copy(iv, initialization_vector, 16);
}

// Encrypts stream buffer using provided password.
//    @param password - intermediate string to generate final encryption key,
//                      this key will be generated using pbkdf2 function.
//    @param pbkdf2_iterations - number of iterations for the pbkdf2 function
//                               to generate encryption key from the provided
//                               password.
//    @return  - error if failed.
Optional<Error> EncryptionStream<encryption::AES128>::Encrypt(const String& password,
	                                                          uint32 pbkdf2_iterations)
{
	// calculate aligned (16 byte) size of the encrypted data
	uint32 encrypted_size = static_cast<uint32>(data.GetNum());
	uint32 alligned_size = encrypted_size + (16 - encrypted_size % 16);

	// allocate temp buffer for encrypted data + 4 bytes to hold
	// size of the encrypted data
	Array<byte> encrypted_data(alligned_size + sizeof(uint32));

	// set first 4 bytes as a size of the encrypted data
	memory::Copy(encrypted_data.GetAddress(), &encrypted_size, sizeof(uint32));

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
	AES128Encryptor encryptor;
	encryptor.EncryptCBC(encrypted_data.GetAddress() + sizeof(uint32),
		                 data.GetAddress(),
		                 alligned_size,
		                 key,
		                 iv);

	// swap current data buffer with encrypted one
	data = encrypted_data;

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
Optional<Error> EncryptionStream<encryption::AES128>::Decrypt(const String& password,
	                                                          uint32 pbkdf2_iterations)
{
	// check current (encrypted) data size - it must contain
	// at least 4 bytes for data size
	if (data.GetNum() < sizeof(uint32))
	{
		String error_message = "AES encrypted buffer size is too small. ";
		error_message += "Buffer must have at least 4 bytes.";
		return Error(error::fail, error_message);
	}

	// check current (encrypted) data size - it must be aligned
	if ((data.GetNum() - sizeof(uint32)) % 16 > 0)
	{
		String error_message = "AES encrypted buffer size is invalid. ";
		error_message += "Buffer must have 16-byte alignment.";
		return Error(error::fail, error_message);
	}

	// extract encrypted data size from the buffer
	uint32 decrypted_size;
	memory::Copy(&decrypted_size, data.GetAddress(), sizeof(uint32));

	// check current (encrypted) data size - it must be sufficient
	// for the size provided in first 4 bytes
	if (data.GetNum() - sizeof(uint32) < decrypted_size)
	{
		String error_message = "AES encrypted buffer size is too small. ";
		return Error(error::fail, error_message);
	}

	// allocate temp buffer for decrypted data
	Array<byte> decrypted_data(data.GetNum());

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

	// decrypt blocks using one more temporary buffer with pure size (-4 bytes),
	// this is done because cbc implementation of the AES128 encryption uses
	// input buffer for intermediate calculations
	Array<byte> input_buffer(data.GetNum() - sizeof(uint32));
	memory::Copy(input_buffer.GetAddress(),
	             data.GetAddress() + sizeof(uint32),
	             input_buffer.GetNum());

	// decrypt data
	AES128Encryptor encryptor;
	size_t blocks = input_buffer.GetNum() / 16;
	for (size_t i = 0; i < blocks; i++)
	{
		const byte* bkey = i == 0 ? key : nullptr;
		const byte* biv = i == 0 ? iv : nullptr;
		encryptor.DecryptCBC(decrypted_data.GetAddress() + i * 16,
			                 input_buffer.GetAddress() + i * 16,
			                 16,
			                 bkey,
			                 biv);
	}

	// swap current data buffer with decrypted one
	data = decrypted_data;

	// crop useless tail
	data.Resize(decrypted_size);

	// success
	return Optional<Error>();
}

// Default initialization vector for AES128 encryption.
// Represents 0x01, 0x02, 0x03 ... 0x0f sequence.
const byte EncryptionStream<encryption::AES128>::skIv[16] = { 0x00, 0x01, 0x02, 0x03,
                                                              0x04, 0x05, 0x06, 0x07,
                                                              0x08, 0x09, 0x0a, 0x0b,
                                                              0x0c, 0x0d, 0x0e, 0x0f };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
