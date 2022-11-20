//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//
// Note - This code makes a few assumptions about how your machine behaves -
//
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
//
// And it has a few limitations -
//
// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.
//----------------------------------------------------------------------------//
// Calculates MurmurHash-2 32-bit hash.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 32-bit seed.
//    @return - 32-bit hash.
uint32 MurmurHash2(const void* key,
                   int len,
                   uint32 seed);

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
                     uint64 seed);

// Same as MurmurHash64A, but for 32-bit platforms.
// 64-bit hash for 64-bit platforms.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 64-bit seed.
//    @return - 64-bit hash.
uint64 MurmurHash64B(const void* key,
                     int len,
                     uint64 seed);

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
uint32 MurmurHash2A(const void* key,
                    int len,
                    uint32 seed);

// MurmurHashNeutral2, by Austin Appleby.
// Same as MurmurHash2, but endian- and alignment-neutral.
// Half the speed though, alas.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 32-bit seed.
//    @return - 32-bit hash.
uint32 MurmurHashNeutral2(const void* key,
                          int len,
                          uint32 seed);

// MurmurHashAligned2, by Austin Appleby.
// Same algorithm as MurmurHash2, but only does aligned reads - should be safer
// on certain platforms. 
// Performance will be lower than MurmurHash2.
//    @param key - pointer to the key.
//    @param len - size of the @key.
//    @param seed - 32-bit seed.
//    @return - 32-bit hash.
uint32 MurmurHashAligned2(const void* key,
                          int len,
                          uint32 seed);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
