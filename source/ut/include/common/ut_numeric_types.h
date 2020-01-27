//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_def.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Integers
#if CPP_STANDARD >= 2011
#include <cstdint>
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
#else
typedef signed char 	 int8;
typedef signed short int int16;
typedef signed int 	     int32;
typedef long long 	     int64;

typedef unsigned char 	    uint8;
typedef unsigned short int 	uint16;
typedef unsigned int 	    uint32;
typedef unsigned long long 	uint64;
#endif
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
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//