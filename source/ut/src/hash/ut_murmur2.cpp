//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "hash/ut_murmur2.h"
//----------------------------------------------------------------------------//
#if defined(_MSC_VER)
#define MURMUR2_BIG_CONSTANT(x) (x)
#else
#define MURMUR2_BIG_CONSTANT(x) (x##LLU)
#endif
#define MURMUR2_MIX(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }
#define MURMUR2_MIX_A(h,k,m) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Calculates MurmurHash-2 32-bit hash.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 32-bit seed.
//    @return - 32-bit hash.
uint32 MurmurHash2(const void* key,
                   int len,
                   uint32 seed)
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const uint32 m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	uint32 h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while (len >= 4)
	{
		uint32 k = *(uint32*)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array

	switch (len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

//----------------------------------------------------------------------------//
// MurmurHash2, 64-bit versions, by Austin Appleby
// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
// and endian-ness issues if used across multiple platforms.
// 64-bit hash for 64-bit platforms.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 64-bit seed.
//    @return - 64-bit hash.
uint64 MurmurHash64A(const void* key,
                     int len,
                     uint64 seed)
{
	const uint64 m = MURMUR2_BIG_CONSTANT(0xc6a4a7935bd1e995);
	const int r = 47;

	uint64 h = seed ^ (len * m);

	const uint64 * data = (const uint64 *)key;
	const uint64 * end = data + (len / 8);

	while (data != end)
	{
		uint64 k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch (len & 7)
	{
	case 7: h ^= uint64(data2[6]) << 48;
	case 6: h ^= uint64(data2[5]) << 40;
	case 5: h ^= uint64(data2[4]) << 32;
	case 4: h ^= uint64(data2[3]) << 24;
	case 3: h ^= uint64(data2[2]) << 16;
	case 2: h ^= uint64(data2[1]) << 8;
	case 1: h ^= uint64(data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

//----------------------------------------------------------------------------//
// Same as MurmurHash64A, but for 32-bit platforms.
// 64-bit hash for 64-bit platforms.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 64-bit seed.
//    @return - 64-bit hash.
uint64 MurmurHash64B(const void * key, int len, uint64 seed)
{
	const uint32 m = 0x5bd1e995;
	const int r = 24;

	uint32 h1 = uint32(seed) ^ len;
	uint32 h2 = uint32(seed >> 32);

	const uint32 * data = (const uint32 *)key;

	while (len >= 8)
	{
		uint32 k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		uint32 k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if (len >= 4)
	{
		uint32 k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch (len)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
		h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	uint64 h = h1;

	h = (h << 32) | h2;

	return h;
}

//----------------------------------------------------------------------------//
// MurmurHash2A, by Austin Appleby.
// This is a variant of MurmurHash2 modified to use the Merkle-Damgard 
// construction. Bulk speed should be identical to Murmur2, small-key speed 
// will be 10%-20% slower due to the added overhead at the end of the hash.
// This variant fixes a minor issue where null keys were more likely to
// collide with each other than expected, and also makes the function
// more amenable to incremental implementations.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 32-bit seed.
//    @return - 32-bit hash.
uint32 MurmurHash2A(const void * key, int len, uint32 seed)
{
	const uint32 m = 0x5bd1e995;
	const int r = 24;
	uint32 l = len;

	const unsigned char * data = (const unsigned char *)key;

	uint32 h = seed;

	while (len >= 4)
	{
		uint32 k = *(uint32*)data;

		MURMUR2_MIX(h, k);

		data += 4;
		len -= 4;
	}

	uint32 t = 0;

	switch (len)
	{
	case 3: t ^= data[2] << 16;
	case 2: t ^= data[1] << 8;
	case 1: t ^= data[0];
	};

	MURMUR2_MIX(h, t);
	MURMUR2_MIX(h, l);

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

//----------------------------------------------------------------------------//
// MurmurHashNeutral2, by Austin Appleby.
// Same as MurmurHash2, but endian- and alignment-neutral.
// Half the speed though, alas.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 32-bit seed.
//    @return - 32-bit hash.
uint32 MurmurHashNeutral2(const void* key,
                          int len,
                          uint32 seed)
{
	const uint32 m = 0x5bd1e995;
	const int r = 24;

	uint32 h = seed ^ len;

	const unsigned char * data = (const unsigned char *)key;

	while (len >= 4)
	{
		uint32 k;

		k = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	switch (len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

//----------------------------------------------------------------------------//
// MurmurHashAligned2, by Austin Appleby.
// Same algorithm as MurmurHash2, but only does aligned reads - should be safer
// on certain platforms. 
// Performance will be lower than MurmurHash2.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 32-bit seed.
//    @return - 32-bit hash.
uint32 MurmurHashAligned2(const void * key, int len, uint32 seed)
{
	const uint32 m = 0x5bd1e995;
	const int r = 24;

	const unsigned char * data = (const unsigned char *)key;

	uint32 h = seed ^ len;

	int align = (uint64)data & 3;

	if (align && (len >= 4))
	{
		// Pre-load the temp registers

		uint32 t = 0, d = 0;

		switch (align)
		{
		case 1: t |= data[2] << 16;
		case 2: t |= data[1] << 8;
		case 3: t |= data[0];
		}

		t <<= (8 * align);

		data += 4 - align;
		len -= 4 - align;

		int sl = 8 * (4 - align);
		int sr = 8 * align;

		// Mix

		while (len >= 4)
		{
			d = *(uint32 *)data;
			t = (t >> sr) | (d << sl);

			uint32 k = t;

			MURMUR2_MIX_A(h, k, m);

			t = d;

			data += 4;
			len -= 4;
		}

		// Handle leftover data in temp registers

		d = 0;

		if (len >= align)
		{
			switch (align)
			{
			case 3: d |= data[2] << 16;
			case 2: d |= data[1] << 8;
			case 1: d |= data[0];
			}

			uint32 k = (t >> sr) | (d << sl);
			MURMUR2_MIX_A(h, k, m);

			data += align;
			len -= align;

			//----------
			// Handle tail bytes

			switch (len)
			{
			case 3: h ^= data[2] << 16;
			case 2: h ^= data[1] << 8;
			case 1: h ^= data[0];
				h *= m;
			};
		}
		else
		{
			switch (len)
			{
			case 3: d |= data[2] << 16;
			case 2: d |= data[1] << 8;
			case 1: d |= data[0];
			case 0: h ^= (t >> sr) | (d << sl);
				h *= m;
			}
		}

		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;

		return h;
	}
	else
	{
		while (len >= 4)
		{
			uint32 k = *(uint32 *)data;

			MURMUR2_MIX_A(h, k, m);

			data += 4;
			len -= 4;
		}

		//----------
		// Handle tail bytes

		switch (len)
		{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= m;
		};

		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;

		return h;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
