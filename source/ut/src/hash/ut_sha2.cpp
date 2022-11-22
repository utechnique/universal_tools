//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "hash/ut_sha2.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
#define SHA2_SHFR(x, n)    (x >> n)
#define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
#define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}
#define SHA2_PACK32(str, x)                   \
{                                             \
    *(x) =   ((uint32) *((str) + 3)      )    \
           | ((uint32) *((str) + 2) <<  8)    \
           | ((uint32) *((str) + 1) << 16)    \
           | ((uint32) *((str) + 0) << 24);   \
}
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Message is broken in 512-bit chunks, each chunk
// consists of 8 operating blocks, so 1 block = 64 bits.
const uint32 Sha256Function::skBlockSize = (512 / 8);

// Size of the final hash in bytes.
const uint32 Sha256Function::skDigestSize = (256 / 8);

// first 32 bits of the fractional parts of the cube roots
// of the first 64 primes 2..311
const uint32 Sha256Function::skSha256ConstantTable[64] = //UL = uint32
{ 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
  0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
  0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
  0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
  0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
  0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
  0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
  0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
  0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

//----------------------------------------------------------------------------//
// Constructor, initializes hash values and other members.
Sha256Function::Sha256Function()
{
	Reset();
}

// Resets hashing result.
void Sha256Function::Reset()
{
	hash[0] = 0x6a09e667;
	hash[1] = 0xbb67ae85;
	hash[2] = 0x3c6ef372;
	hash[3] = 0xa54ff53a;
	hash[4] = 0x510e527f;
	hash[5] = 0x9b05688c;
	hash[6] = 0x1f83d9ab;
	hash[7] = 0x5be0cd19;
	length = 0;
	total_length = 0;
}

// Transforms provided data into 256-bit hash.
//    @param message - pointer to the first byte of the data.
//    @param block_nb - number of blocks
void Sha256Function::Transform(const byte* message, uint32 block_nb)
{
	uint32 w[64];
	uint32 wv[8];
	uint32 t1, t2;
	const byte* sub_block;
	int i;
	int j;

	for (i = 0; i < (int)block_nb; i++)
	{
		sub_block = message + (i << 6);

		for (j = 0; j < 16; j++)
		{
			SHA2_PACK32(&sub_block[j << 2], &w[j]);
		}

		for (j = 16; j < 64; j++)
		{
			w[j] = SHA256_F4(w[j - 2]) + w[j - 7] + SHA256_F3(w[j - 15]) + w[j - 16];
		}

		for (j = 0; j < 8; j++)
		{
			wv[j] = hash[j];
		}

		for (j = 0; j < 64; j++)
		{
			t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
				+ skSha256ConstantTable[j] + w[j];
			t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
			wv[7] = wv[6];
			wv[6] = wv[5];
			wv[5] = wv[4];
			wv[4] = wv[3] + t1;
			wv[3] = wv[2];
			wv[2] = wv[1];
			wv[1] = wv[0];
			wv[0] = t1 + t2;
		}

		for (j = 0; j < 8; j++)
		{
			hash[j] += wv[j];
		}
	}
}

// Final stage to generate 256-bit hash.
//    @param digest - pointer to the first byte of destination
//                    buffer for the hash.
void Sha256Function::FinalizeHash(byte* digest)
{
	uint32 block_nb;
	uint32 pm_len;
	uint32 len_b;
	int i;
	block_nb = (1 + ((skBlockSize - 9) < (length % skBlockSize)));
	len_b = (total_length + length) << 3;
	pm_len = block_nb << 6;
	memory::Set(block + length, 0, pm_len - length);
	block[length] = 0x80;
	SHA2_UNPACK32(len_b, block + pm_len - 4);
	Transform(block, block_nb);
	for (i = 0; i < 8; i++)
	{
		SHA2_UNPACK32(hash[i], &digest[i << 2]);
	}
}

// Generates 256-bit hash from provided data.
//    @param message - pointer to the first byte of the data.
//    @param len - size of the provided data in bytes.
//    @param digest - pointer to the first byte of destination
//                    buffer for the hash.
void Sha256Function::Calculate(const byte* message, uint32 len, byte* digest)
{
	Reset();
	uint32 block_nb;
	uint32 new_len, rem_len, tmp_len;
	const byte* shifted_message;
	tmp_len = skBlockSize - length;
	rem_len = len < tmp_len ? len : tmp_len;
	memory::Copy(&block[length], message, rem_len);
	if (length + len < skBlockSize)
	{
		length += len;
	}
	else
	{
		new_len = len - rem_len;
		block_nb = new_len / skBlockSize;
		shifted_message = message + rem_len;
		Transform(block, 1);
		Transform(shifted_message, block_nb);
		rem_len = new_len % skBlockSize;
		memory::Copy(block, &shifted_message[block_nb << 6], rem_len);
		length = rem_len;
		total_length += (block_nb + 1) << 6;
	}
	FinalizeHash(digest);
}

//----------------------------------------------------------------------------//
// Text variant of sha-256 function.
//    @param input - string to generate hash from.
//    @return - 256-bit hash in text form.
String Sha256(const String& input)
{
	// buffer for sha256 hash
	byte digest[Sha256Function::skDigestSize];

	// calculate hash
	Sha256Function sha256;
	sha256.Calculate((byte*)input.GetAddress(), (uint32)input.Length(), digest);

	// allocate string memory
	String out(2 * Sha256Function::skDigestSize);

	// convert to text form
	for (int i = 0; i < Sha256Function::skDigestSize; i++)
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
