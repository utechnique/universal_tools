//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_endianness.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(endian)
//----------------------------------------------------------------------------//
// A simple way to detect endianness is to check first (or last) byte of a
// bigger integer variable after initialization.
order DetectEndianness()
{
	uint32 u = 0x00000001;
	uint8 c = *(uint8*)&u;
	return c == 0x01 ? little : big;
}

// Use this function to know endianness order of the current platform.
order GetNative()
{
	static order endianness_order = DetectEndianness();
	return endianness_order;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(endian)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//