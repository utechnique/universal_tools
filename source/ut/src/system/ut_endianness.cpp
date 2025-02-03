//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_endianness.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(endianness)
//----------------------------------------------------------------------------//
// A simple way to detect endianness is to check first (or last) byte of a
// bigger integer variable after initialization.
Order DetectEndianness()
{
	uint32 u = 0x00000001;
	uint8 c = *(uint8*)&u;
	return c == 0x01 ? Order::little : Order::big;
}

// Use this function to know endianness order of the current platform.
Order GetNative()
{
	static const Order endianness_order = DetectEndianness();
	return endianness_order;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(endian)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//