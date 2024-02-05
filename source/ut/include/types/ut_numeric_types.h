//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "platforms/ut_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Integers
#include <cstdint>
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint32 uint;

//----------------------------------------------------------------------------//
// Pointers
#if UT_PLATFORM_64BITS
typedef uint64 uptr;
typedef int64 iptr;
#elif UT_PLATFORM_32BITS
typedef uint32 uptr;
typedef int32 iptr;
#endif

//----------------------------------------------------------------------------//
// Words and byte
typedef uint8  byte;
typedef uint16 word;
typedef uint32 dword;
typedef uint64 qword;

//----------------------------------------------------------------------------//
// Checks whether template argument is an integral type.
template<typename NumericType>
struct IsIntegral
{
	static constexpr bool value = true;
};

// ut::IsIntegral returns false for floating point types.
template<> struct IsIntegral<float> { static constexpr bool value = false; };
template<> struct IsIntegral<double> { static constexpr bool value = false; };
template<> struct IsIntegral<long double> { static constexpr bool value = false; };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//