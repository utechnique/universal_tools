//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(memory)
//----------------------------------------------------------------------------//
// Copies specified number of bytes (@size) from the object pointed to by @src
// to the object pointed to by @dst. Both objects are reinterpreted as arrays of
// unsigned char.
//    @param dst - pointer to the memory location to copy to.
//    @param src - pointer to the memory location to copy from.
//    @param size - number of bytes to copy.
//    @return - @dst value.
//
// This implementation is just a wrapper over std::memcpy() function. Because
// std::memcpy is meant to be the fastest library routine for memory-to-memory
// copy. Compiler can have built-in intrinsic variant, so there is no sense in
// trying to make something faster than this.
inline void* Copy(void *dst, const void *src, size_t size)
{
	return memcpy(dst, src, size);
}

// Converts the value @val to unsigned char and copies it into each of the first
// @size characters of the object pointed to by @dst.
//    @param dst - pointer to the object to fill.
//    @param val - fill byte.
//    @param size - number of bytes to fill.
//    @return - @dst value.
//
// This implementation is just a wrapper over std::memset() function for the
// same reason as ut::memory::Copy() is a wrapper over std::memcpy.
inline void* Set(void *dst, int val, size_t size)
{
	return memset(dst, val, size);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(memory)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//