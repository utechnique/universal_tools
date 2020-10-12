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

// Allocates a block of size bytes of memory, returning a pointer to the
// beginning of the block. The content of the newly allocated block of
// memory is not initialized, remaining with indeterminate values.
//    @param size - size of the memory block, in bytes.
//    @return - on success, a pointer to the memory block allocated by the
//              function; if the function failed to allocate the requested
//              block of memory, a null pointer is returned.
inline void* Allocate(size_t size)
{
	return malloc(size);
}

// A block of memory previously allocated by a call to ut::memory::Allocate
// is deallocated, making it available again for further allocations.
inline void Deallocate(void* ptr)
{
	return free(ptr);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(memory)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//