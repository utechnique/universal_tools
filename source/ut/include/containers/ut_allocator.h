//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "system/ut_memory.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_class.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Default allocator. An allocator must have Allocate and Deallocate methods.
template<typename ElementType>
class DefaultAllocator
{
public:
	ElementType* Allocate(size_t n)
	{
		return static_cast<ElementType*>(ut::memory::Allocate(n * sizeof(ElementType)));
	}

	void Deallocate(ElementType* addr, size_t n)
	{
		ut::memory::Deallocate(addr);
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//