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

// Default preallocator. A preallocator calculates how many elements must be
// preallocated (or deallocated) for future uses.
template<size_t inc_factor, size_t dec_factor>
struct DefaultPreallocator
{
public:
	void SetCapacityReduction(bool status)
	{
		can_decrease = status;
	}

protected:
	size_t operator()(size_t elements, size_t current_capacity) const
	{
		const size_t inc_capacity = elements * inc_factor;
		const bool increase = elements > current_capacity;
		const bool reduce = can_decrease && (inc_capacity <= current_capacity / dec_factor);
		if (increase || reduce)
		{
			return inc_capacity;
		}

		return current_capacity;
	}

private:
	bool can_decrease = true;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//