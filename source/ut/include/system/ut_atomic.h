//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "system/ut_interlocked.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Each instantiation and full specialization of the ut::Atomic template defines
// an atomic type. If one thread writes to an atomic object while another
// thread reads from it, the behavior is well-defined.
template<typename T> class Atomic {};

// Specialization of the ut::Atomic tempate for 'int32' type.
template<> class Atomic<int32> : public NonCopyable
{
public:
	Atomic(int32 ini_val = 0) : var(ini_val)
	{}

	inline int32 Increment()
	{
		return atomics::interlocked::Increment(&var);
	}

	inline int32 Decrement()
	{
		return atomics::interlocked::Decrement(&var);
	}

	inline int32 Add(int32 amount)
	{
		return atomics::interlocked::Add(&var, amount);
	}

	inline int32 Exchange(int32 exchange)
	{
		return atomics::interlocked::Exchange(&var, exchange);
	}

	inline int32 CompareExchange(int32 exchange, int32 comparand)
	{
		return atomics::interlocked::CompareExchange(&var, exchange, comparand);
	}

	inline int32 Read()
	{
		return atomics::interlocked::Read(&var);
	}

	inline void Store(int32 val)
	{
		return atomics::interlocked::Store(&var, val);
	}

private:
	int32 var;
};

// Specialization of the ut::Atomic tempate for 'bool' type.
template<> class Atomic<bool> : public NonCopyable
{
public:
	Atomic(bool ini_val = false) : var(ini_val ? 1 : 0)
	{}

	inline bool Increment()
	{
		Store(true);
		return true;
	}

	inline bool Decrement()
	{
		Store(false);
		return false;
	}

	inline bool Add(bool amount)
	{
		return CompareExchange(amount, false);
	}

	inline bool Exchange(bool exchange)
	{
		return atomics::interlocked::Exchange(&var, exchange ? 1 : 0) != 0;
	}

	inline bool CompareExchange(bool exchange, bool comparand)
	{
		return atomics::interlocked::CompareExchange(&var, exchange ? 1 : 0, comparand ? 1 : 0) != 0;
	}

	inline bool Read()
	{
		return atomics::interlocked::Read(&var) != 0;
	}

	inline void Store(bool val)
	{
		return atomics::interlocked::Store(&var, val ? 1 : 0);
	}

private:
	int32 var;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//