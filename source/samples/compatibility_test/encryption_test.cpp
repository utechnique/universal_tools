//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "encryption_test.h"
//----------------------------------------------------------------------------//
EncryptionTestUnit::EncryptionTestUnit() : TestUnit("ENCRYPTION")
{
	tasks.Add(ut::MakeUnique<HashTask>());
	tasks.Add(ut::MakeUnique<AesTask>());
	tasks.Add(ut::MakeUnique<XorTask>());
	tasks.Add(ut::MakeUnique<Base64Task>());
}

//----------------------------------------------------------------------------//
HashTask::HashTask() : TestTask("Hash")
{ }

void HashTask::Execute()
{
	ut::String sha_test_0 = ut::Sha256("test");
	ut::String sha_test_1 = ut::Sha256("test1");
	ut::String sha_test_2 = ut::Sha256("test");

	ut::String hmac_sha_test_0 = ut::HmacSha256("key", "The quick brown fox jumps over the lazy dog");

	ut::String pbkdf2_sha_test_0 = ut::Pbkdf2Sha256("password", "salt", 1024, 11);
	ut::String pbkdf2_sha_test_1 = ut::Pbkdf2Sha256("password", "salt", 1023, 11);
	ut::String pbkdf2_sha_test_2 = ut::Pbkdf2Sha256("password", "salr", 1023, 11);

	report += ut::cret + ut::String("  Sha256 test: ") + ut::cret;
	report += ut::String("    1: ") + sha_test_0 + ut::cret;
	report += ut::String("    2: ") + sha_test_1 + ut::cret;
	report += ut::String("    3: ") + sha_test_2 + ut::cret;

	if (sha_test_0 != sha_test_2 || sha_test_1 == sha_test_0 || sha_test_1 == sha_test_2)
	{
		report += ut::String("ERROR. Invalid sha256 behaviour.");
		failed_test_counter.Increment();
	}

	report += ut::cret + ut::String("  HmacSha256 test: ") + ut::cret;
	report += ut::String("    1: ") + hmac_sha_test_0 + ut::cret;
	
	if (hmac_sha_test_0 != "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8")
	{
		report += ut::String("ERROR. Invalid hmac-sha256 behaviour.");
		failed_test_counter.Increment();
	}

	report += ut::cret + ut::String("  Pbkdf2Sha256 test: ") + ut::cret;
	report += ut::String("    1: ") + pbkdf2_sha_test_0 + ut::cret;
	report += ut::String("    2: ") + pbkdf2_sha_test_1 + ut::cret;
	report += ut::String("    3: ") + pbkdf2_sha_test_2 + ut::cret;
}

//----------------------------------------------------------------------------//
AesTask::AesTask() : TestTask("AES")
{ }

void AesTask::Execute()
{
	ut::EncryptionStream<ut::encryption::AES128> stream;
	ut::String password("password");
	ut::String original_string("The quick brown fox jumps over the lazy dog");

	stream << original_string;

	stream.Encrypt(password);

	const ut::Array<ut::byte>& buffer = stream.GetBuffer();
	ut::String encrypted_string(buffer.Count());
	ut::memory::Copy(encrypted_string.GetAddress(), buffer.GetAddress(), buffer.Count());

	for (size_t i = 0; i < encrypted_string.Length(); i++)
	{
		if (encrypted_string[i] < 32 || encrypted_string[i] > 127)
		{
			encrypted_string[i] = 'X'; // do not break console output
		}
	}

	ut::Optional<ut::Error> decryption_error = stream.Decrypt(password);
	if (decryption_error)
	{
		report += ut::String("ERROR. Failed to decrypt AES128 buffer.") + ut::cret;
		report += decryption_error->GetDesc() + ut::cret;
		failed_test_counter.Increment();
	}

	ut::String decrypted_string(buffer.Count());
	ut::memory::Copy(decrypted_string.GetAddress(), buffer.GetAddress(), buffer.Count());

	for (size_t i = 0; i < decrypted_string.Length(); i++)
	{
		if (decrypted_string[i] && (decrypted_string[i] < 32 || decrypted_string[i] > 127))
		{
			decrypted_string[i] = 'X'; // do not break console output
		}
	}

	report += ut::cret + ut::String("  original string: ") + original_string + ut::cret;
	report += ut::String("  encrypted string: ") + encrypted_string + ut::cret;
	report += ut::String("  decrypted string: ") + decrypted_string + ut::cret;

	if (original_string != decrypted_string)
	{
		report += ut::String("ERROR. Invalid aes128 behaviour.");
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
XorTask::XorTask() : TestTask("XOR")
{ }

void XorTask::Execute()
{
	ut::EncryptionStream<ut::encryption::XOR> stream;
	ut::String password("password");
	ut::String original_string("The quick brown fox jumps over the lazy dog");

	stream << original_string;

	stream.Encrypt(password);

	const ut::Array<ut::byte>& buffer = stream.GetBuffer();
	ut::String encrypted_string(buffer.Count());
	ut::memory::Copy(encrypted_string.GetAddress(), buffer.GetAddress(), buffer.Count());

	for (size_t i = 0; i < encrypted_string.Length(); i++)
	{
		if (encrypted_string[i] < 32 || encrypted_string[i] > 127)
		{
			encrypted_string[i] = 'X'; // do not break console output
		}
	}

	stream.Decrypt(password);

	ut::String decrypted_string(buffer.Count());
	ut::memory::Copy(decrypted_string.GetAddress(), buffer.GetAddress(), buffer.Count());

	for (size_t i = 0; i < decrypted_string.Length(); i++)
	{
		if (decrypted_string[i] && (decrypted_string[i] < 32 || decrypted_string[i] > 127))
		{
			decrypted_string[i] = 'X'; // do not break console output
		}
	}

	report += ut::cret + ut::String("  original string: ") + original_string + ut::cret;
	report += ut::String("  encrypted string: ") + encrypted_string + ut::cret;
	report += ut::String("  decrypted string: ") + decrypted_string + ut::cret;

	if (original_string != decrypted_string)
	{
		report += ut::String("ERROR. Invalid xor encryption behaviour.");
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
Base64Task::Base64Task() : TestTask("Base64")
{ }

void Base64Task::Execute()
{
	ut::Array<ut::byte> data(256);
	for (size_t i = 0; i < 256; i++)
	{
		data[i] = static_cast<ut::byte>(i);
	}

	ut::String base64 = ut::EncodeBase64(data.GetAddress(), data.GetSize());
	report += base64 + ut::cret;

	data = ut::DecodeBase64(base64);
	if (data.GetSize() != 256)
	{
		report += ut::String("FAIL. The data has invalid size after decode.");
		failed_test_counter.Increment();
		return;
	}

	for (size_t i = 0; i < 256; i++)
	{
		if (data[i] != i)
		{
			report += ut::String("FAIL. The data doesn\'t match after encode/decode.");
			failed_test_counter.Increment();
			return;
		}
	}

	report += ut::String("Ok.") + ut::cret;
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//