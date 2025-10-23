//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "system/ut_time.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(rng)
//----------------------------------------------------------------------------//
// Supported random generator engines.
enum class Algorithm
{
	mt19937
};

//----------------------------------------------------------------------------//
// Every random generator must have at least a constructor and Seed() member
// function accepting the seeding value, Min() and Max() member functions and a
// member operator() returning a random value.
template<Algorithm algorithm = Algorithm::mt19937>
class Generator;

//----------------------------------------------------------------------------//
// Mersenne Twister 19937 generator.
template<>
class Generator<Algorithm::mt19937>
{
public:
	// Constructs a mersenne twister generator object, and initializes its
	// internal state sequence to pseudo-random values.
	Generator(uint32 seed = static_cast<uint32>(time::GetTime<time::Unit::second>() % Max())) noexcept;

	// Re-initializes the internal state sequence to pseudo-random value
	// applying a linear random generator on a single value.
	//    @param seed - a seeding value. An entire state sequence is generated
	//                  from this value using a linear random generator.
	void Seed(uint32 seed);

	// Returns a new random number.
	uint32 operator ()();

	// Returns the minimum value potentially returned by member operator(),
	// which for mersenne twister is always zero.
	static constexpr uint32 Min()
	{
		return 0;
	}

	// Returns the maximum value potentially returned by member operator(), 
	// which for mersenne twister is 2^32 - 1 (for current 32-bit variant).
	static constexpr uint32 Max()
	{
		return 4294967295;
	}

public:
	// Initializes generator state with a seeding value.
	//    @param seed - a seeding value.
	void Initialize(const uint32 seed);

	// Generates skN new values in the state.
	void Reload();

	// Helper function to extract the highest bit.
	static constexpr uint32 HiBit(const uint32 u)
	{
		return u & 0x80000000UL;
	}

	// Helper function to extract the lowest bit.
	static constexpr uint32 LoBit(const uint32 u)
	{
		return u & 0x00000001UL;
	}

	// Helper function to extract the lowest bits.
	static constexpr uint32 LoBits(const uint32 u)
	{
		return u & 0x7fffffffUL;
	}

	// Helper function to mix the highest and the lowest bits.
	static constexpr uint32 MixBits(const uint32 u, const uint32 v)
	{
		return HiBit(u) | LoBits(v);
	}

	// Helper magic function.
	static constexpr uint32 Magic(const uint32 u)
	{
		return LoBit(u) ? 0x9908b0dfUL : 0x0UL;
	}

	// The twist operation.
	static constexpr uint32 Twist(const uint32 m, const uint32 s0, const uint32 s1)
	{
		return m ^ (MixBits(s0, s1) >> 1) ^ Magic(s1);
	}

	// The length of the state vector.
	static constexpr int skN = 624;

	// The period parameter.
	static constexpr int skM = 397;

	// The internal state.
	uint32 state[skN];

	// The next value to get from the state.
	uint32 next;

	// Indicates how many unused values does the state have.
	int left;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(rng)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//