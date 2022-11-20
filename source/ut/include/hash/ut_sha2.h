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
// using sha256 algorithm.
class Sha256Function
{
protected:
	// First 32 bits of the fractional parts of the cube roots
	// of the first 64 primes 2..311
	const static uint32 skSha256ConstantTable[64];

	// Message is broken in 512-bit chunks, each chunk
	// consists of 8 operating blocks, so 1 block = 64 bits.
	static const uint32 skBlockSize;
public:
	// Constructor, initializes hash values and other members.
	Sha256Function();

	// Generates 256-bit hash from provided data.
	//    @param message - pointer to the first byte of the data.
	//    @param len - size of the provided data in bytes.
	//    @param digest - pointer to the first byte of destination
	//                    buffer (32 bytes) for the hash.
	void Calculate(const byte* message, uint32 len, byte* digest);

	// Size of the final hash in bytes.
	static const uint32 skDigestSize;

protected:
	// Resets hashing result.
	void Reset();

	// Transforms provided data into 256-bit hash.
	//    @param message - pointer to the first byte of the data.
	//    @param block_nb - number of blocks
	void Transform(const byte* message, uint32 block_nb);

	// Final stage to generate 256-bit hash.
	//    @param digest - pointer to the first byte of destination
	//                    buffer for the hash.
	void FinalizeHash(byte* digest);

	// Intermediate values.
	uint32 total_length;
	uint32 length;
	byte block[128];

	// First 32 bits of the fractional parts of the
	// square roots of the first 8 primes 2..19
	uint32 hash[8];
};

//----------------------------------------------------------------------------//
// Text variant of sha-256 function.
//    @param input - string to generate hash from.
//    @return - 256-bit hash in text form.
String Sha256(const String& input);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
