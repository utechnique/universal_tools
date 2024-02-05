//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "preprocessor/ut_def.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Helper struct to count combinations.
template<size_t first, size_t... dimensions>
struct PermutationCounter
{
	static constexpr size_t element_size = PermutationCounter<dimensions...>::size;
	static constexpr size_t size = first * element_size;

	template<typename... Coordinates>
	static constexpr size_t GetId(size_t id, Coordinates... coordinates)
	{
		return id * element_size + PermutationCounter<dimensions...>::GetId(coordinates...);
	}

	template<size_t axis>
	static constexpr size_t GetCoordinate(size_t id)
	{
		return axis == 0 ? (id / element_size) : PermutationCounter<dimensions...>::template GetCoordinate<axis - 1>(id % element_size);
	}
};

// Specialization fo the last dimension.
template<size_t last> struct PermutationCounter<last>
{
	static constexpr size_t size = last;

	static constexpr size_t GetId(size_t id)
	{
		return id;
	}

	template<size_t axis>
	static constexpr size_t GetCoordinate(size_t id)
	{
		return id;
	}
};

// ut::Grid enumerates all possible combinations of the provided
// multidimensional discrete coordinates and assigns an index to all of them.
// For example it can translate such 2d system:
//  0 0
//  1 1   into 1d sequence: (0,0) (0,1) (1,0) (1,1) (2,0) (2,1)
//  2
template<size_t... dimensions>
struct Grid
{
	// The number of combinations in this grid.
	static constexpr size_t size = PermutationCounter<dimensions...>::size;

	// Returns the index of the element defined by its coordinates.
	template<typename... Coordinates>
	static constexpr size_t GetId(Coordinates... coordinates)
	{
		return PermutationCounter<dimensions...>::GetId(coordinates...);
	}

	// Returns a coordinate associated with the provided index.
	template<size_t axis>
	static constexpr size_t GetCoordinate(size_t id)
	{
		return PermutationCounter<dimensions...>::template GetCoordinate<axis>(id);
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//