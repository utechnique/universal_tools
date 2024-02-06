//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "thread/ut_interlocked.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Each instantiation and full specialization of the ut::Atomic template defines
// an atomic type. If one thread writes to an atomic object while another
// thread reads from it, the behavior is well-defined.
template<typename T> class Atomic {};

// Helper class for signed values.
template<typename SignedType>
class AtomicSigned : public NonCopyable
{
public:
	AtomicSigned(SignedType ini_val = 0) : var(ini_val)
	{}

	inline SignedType Increment()
	{
		return atomics::interlocked::Increment(&var);
	}

	inline SignedType Decrement()
	{
		return atomics::interlocked::Decrement(&var);
	}

	inline SignedType Add(SignedType amount)
	{
		return atomics::interlocked::Add(&var, amount);
	}

	inline SignedType Exchange(SignedType exchange)
	{
		return atomics::interlocked::Exchange(&var, exchange);
	}

	inline SignedType CompareExchange(SignedType exchange, SignedType comparand)
	{
		return atomics::interlocked::CompareExchange(&var, exchange, comparand);
	}

	inline SignedType Read()
	{
		return atomics::interlocked::Read(&var);
	}

	inline void Store(SignedType val)
	{
		return atomics::interlocked::Store(&var, val);
	}

	SignedType UnsafeRead()
	{
		return var;
	}

	void UnsafeStore(SignedType val)
	{
		var = val;
	}

private:
	SignedType var;
};

// Helper class for unsigned values which must be reinterpreted from signed ones.
template<typename SignedType, typename UnsignedType>
class AtomicUnsigned : public NonCopyable
{
public:
	AtomicUnsigned(UnsignedType ini_val = 0) : var(ini_val)
	{}

	inline UnsignedType Increment()
	{
		SignedType result = atomics::interlocked::Increment(reinterpret_cast<SignedType*>(&var));
		return *reinterpret_cast<UnsignedType*>(&result);
	}

	inline UnsignedType Decrement()
	{
		SignedType result = atomics::interlocked::Decrement(reinterpret_cast<SignedType*>(&var));
		return *reinterpret_cast<UnsignedType*>(&result);
	}

	inline UnsignedType Add(UnsignedType amount)
	{
		SignedType result = atomics::interlocked::Add(reinterpret_cast<SignedType*>(&var),
		                                              *reinterpret_cast<SignedType*>(&amount));
		return *reinterpret_cast<UnsignedType*>(&result);
	}

	inline UnsignedType Exchange(UnsignedType exchange)
	{
		SignedType result = atomics::interlocked::Exchange(reinterpret_cast<SignedType*>(&var),
		                                                   *reinterpret_cast<SignedType*>(&exchange));
		return *reinterpret_cast<UnsignedType*>(&result);
	}

	inline UnsignedType CompareExchange(UnsignedType exchange, UnsignedType comparand)
	{
		SignedType result = atomics::interlocked::CompareExchange(reinterpret_cast<SignedType*>(&var),
		                                                          *reinterpret_cast<SignedType*>(&exchange),
			                                                      *reinterpret_cast<SignedType*>(&comparand));
		return *reinterpret_cast<UnsignedType*>(&result);
	}

	inline UnsignedType Read()
	{
		SignedType result = atomics::interlocked::Read(reinterpret_cast<SignedType*>(&var));
		return *reinterpret_cast<UnsignedType*>(&result);
	}

	inline void Store(UnsignedType val)
	{
		atomics::interlocked::Store(reinterpret_cast<SignedType*>(&var),
		                            *reinterpret_cast<SignedType*>(&val));
	}

	UnsignedType UnsafeRead()
	{
		return var;
	}

	void UnsafeStore(UnsignedType val)
	{
		var = val;
	}

private:
	UnsignedType var;
};

// Specialization of the ut::Atomic tempate for 'int32' type.
template<> class Atomic<int32> : public AtomicSigned<int32>
{
public:
	Atomic(int32 ini_val = 0) : AtomicSigned<int32>(ini_val)
	{}
};

// Specialization of the ut::Atomic tempate for 'uint32' type.
template<> class Atomic<uint32> : public AtomicUnsigned<int32, uint32>
{
public:
	Atomic(uint32 ini_val = 0) : AtomicUnsigned<int32, uint32>(ini_val)
	{}
};

// Specialization of the ut::Atomic tempate for 'int64' type.
template<> class Atomic<int64> : public AtomicSigned<int64>
{
public:
	Atomic(int64 ini_val = 0) : AtomicSigned<int64>(ini_val)
	{}
};

// Specialization of the ut::Atomic tempate for 'uint64' type.
template<> class Atomic<uint64> : public AtomicUnsigned<int64, uint64>
{
public:
	Atomic(uint64 ini_val = 0) : AtomicUnsigned<int64, uint64>(ini_val)
	{}
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

	bool UnsafeRead()
	{
		return var != 0;
	}

	void UnsafeStore(bool val)
	{
		var = val ? 1 : 0;
	}

private:
	int32 var;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//