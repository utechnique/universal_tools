//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "math/ut_rand.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(rng)
//----------------------------------------------------------------------------//
// Constructs a mersenne twister generator object, and initializes its
// internal state sequence to pseudo-random values.
Generator<Algorithm::mt19937>::Generator(uint32 seed) noexcept
{
	Seed(seed);
}

// Re-initializes the internal state sequence to pseudo-random value
// applying a linear random generator on a single value.
//    @param seed - a seeding value. An entire state sequence is generated
//                  from this value using a linear random generator.
void Generator<Algorithm::mt19937>::Seed(uint32 seed)
{
	Initialize(seed);
	Reload();
}

// Returns a new random number.
uint32 Generator<Algorithm::mt19937>::operator()()
{
	if (left == 0)
	{
		Reload();
	}

	--left;

	uint32 s1 = state[next++];
	s1 ^= (s1 >> 11);
	s1 ^= (s1 << 7) & 0x9d2c5680UL;
	s1 ^= (s1 << 15) & 0xefc60000UL;
	return (s1 ^ (s1 >> 18));
}

// Initializes generator state with a seeding value.
//    @param seed - a seeding value.
void Generator<Algorithm::mt19937>::Initialize(const uint32 seed)
{
	uint32* s = state;
	uint32* r = state;
	int i = 1;
	*s++ = seed & 0xffffffffUL;
	for (; i < skN; ++i)
	{
		*s++ = (1812433253UL * (*r ^ (*r >> 30)) + i) & 0xffffffffUL;
		r++;
	}
}

// Generates skN new values in the state.
void Generator<Algorithm::mt19937>::Reload()
{
	static constexpr int MmN = skM - skN;
	static constexpr int NmM = skN - skM;

	uint32* p = state;
	int i;

	for (i = NmM; i--; ++p)
	{
		*p = Twist(p[skM], p[0], p[1]);
	}

	for (i = skM; --i; ++p)
	{
		*p = Twist(p[MmN], p[0], p[1]);
	}

	*p = Twist(p[MmN], p[0], state[0]);

	left = skN;
	next = 0;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(rng)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
