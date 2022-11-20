//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//----------------------------------------------------------------------------//
// Calculates MurmurHash-3 32-bit hash for 32-bit platforms.
//    @param key - pointer to the key.
//    @param len - size of the @key in bytes.
//    @param seed - 32-bit seed.
//    @param out - pointer to the 32-bit hash to write the result to.
void MurmurHash3_x86_32(const void* key,
                        int len,
                        uint32 seed,
                        void* out);

// Calculates MurmurHash-3 128-bit hash for 32-bit platforms.
//    @param key - pointer to the key.
//    @param len - size of the @key in bytes.
//    @param seed - 32-bit seed.
//    @param out - pointer to the 64-bit hash to write the result to.
void MurmurHash3_x86_128(const void* key,
                         int len,
                         uint32 seed,
                         void* out);

// Calculates MurmurHash-3 128-bit hash for 64-bit platforms.
//    @param key - pointer to the key.
//    @param len - size of the @key in bytes.
//    @param seed - 32-bit seed.
//    @param out - pointer to the 64-bit hash to write the result to.
void MurmurHash3_x64_128(const void* key,
                         int len,
                         uint32 seed,
                         void* out);

// Platform-independent signature.
#if UT_PLATFORM_64BITS
inline void MurmurHash3_128(const void* key,
                            int len,
                            uint32 seed,
                            void* out)
{
	MurmurHash3_x64_128(key, len, seed, out);
}
#else
inline void MurmurHash3_128(const void* key,
                            int len,
                            uint32 seed,
                            void* out)
{
	MurmurHash3_x86_128(key, len, seed, out);
}
#endif

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
