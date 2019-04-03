//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_platform.h"
#include "common/ut_def.h"
#include "common/ut_numeric_types.h"
//----------------------------------------------------------------------------//
#if UT_WINDOWS
	#include <intrin.h>
#endif
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(atomics)
START_NAMESPACE(interlocked)
//----------------------------------------------------------------------------//

static inline int32 Increment(volatile int32* Value)
{
#if UT_WINDOWS
	return (int32)_InterlockedIncrement((long*)Value);
#elif UT_UNIX
	return __sync_fetch_and_add(Value, 1) + 1;
#else
	#error ut::atomics::interlocked::Increment() is not implemented
#endif
}

static inline int64 Increment(volatile int64* Value)
{
#if UT_WINDOWS
	#if UT_PLATFORM_64BITS
		return (int64)::_InterlockedIncrement64((long long*)Value);
	#else
		// No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, OldValue + 1, OldValue) == OldValue)
			{
				return OldValue + 1;
			}
		}
	#endif
#elif UT_UNIX
	return __sync_fetch_and_add(Value, 1) + 1;
#else
	#error ut::atomics::interlocked::Increment() is not implemented
#endif
}

static inline int32 Decrement(volatile int32* Value)
{
#if UT_WINDOWS
	return (int32)::_InterlockedDecrement((long*)Value);
#elif UT_UNIX
	return __sync_fetch_and_sub(Value, 1) - 1;
#else
	#error ut::atomics::interlocked::Decrement() is not implemented
#endif
}

static inline int64 Decrement(volatile int64* Value)
{
#if UT_WINDOWS
	#if UT_PLATFORM_64BITS
		return (int64)::_InterlockedDecrement64((long long*)Value);
	#else
		// No explicit instruction for 64-bit atomic decrement on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, OldValue - 1, OldValue) == OldValue)
			{
				return OldValue - 1;
			}
		}
	#endif
#elif UT_UNIX
	return __sync_fetch_and_sub(Value, 1) - 1;
#else
	#error ut::atomics::interlocked::Decrement() is not implemented
#endif
}

static inline int32 Add(volatile int32* Value, int32 Amount)
{
#if UT_WINDOWS
	return (int32)::_InterlockedExchangeAdd((long*)Value, (long)Amount);
#elif UT_UNIX
	return __sync_fetch_and_add(Value, Amount);
#else
	#error ut::atomics::interlocked::Add() is not implemented
#endif
}

static inline int64 Add(volatile int64* Value, int64 Amount)
{
#if UT_WINDOWS
	#if UT_PLATFORM_64BITS
		return (int64)::_InterlockedExchangeAdd64((int64*)Value, (int64)Amount);
	#else
		// No explicit instruction for 64-bit atomic add on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, OldValue + Amount, OldValue) == OldValue)
			{
				return OldValue + Amount;
			}
		}
	#endif
#elif UT_UNIX
	return __sync_fetch_and_add(Value, Amount);
#else
	#error ut::atomics::interlocked::Add() is not implemented
#endif
}

static inline int32 Exchange(volatile int32* Value, int32 Exchange)
{
#if UT_WINDOWS
	return (int32)::_InterlockedExchange((long*)Value, (long)Exchange);
#elif UT_UNIX
	return __sync_lock_test_and_set(Value, Exchange);
#else
	#error ut::atomics::interlocked::Exchange() is not implemented
#endif
}

static inline int64 Exchange(volatile int64* Value, int64 Exchange)
{
#if UT_WINDOWS
	#if UT_PLATFORM_64BITS
		return (int64)::_InterlockedExchange64((long long*)Value, (long long)Exchange);
	#else
		// No explicit instruction for 64-bit atomic exchange on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, Exchange, OldValue) == OldValue)
			{
				return OldValue;
			}
		}
	#endif
#elif UT_UNIX
	return __sync_lock_test_and_set(Value, Exchange);
#else
	#error ut::atomics::interlocked::Exchange() is not implemented
#endif
}

static inline void* ExchangePtr(void** Dest, void* Exchange)
{
#if UT_WINDOWS
	#if UT_PLATFORM_64BITS
		return (void*)::_InterlockedExchange64((int64*)(Dest), (int64)(Exchange));
	#else
		return (void*)::_InterlockedExchange((long*)(Dest), (long)(Exchange));
	#endif
#elif UT_UNIX
	return __sync_lock_test_and_set(Dest, Exchange);
#else
	#error ut::atomics::interlocked::ExchangePtr() is not implemented
#endif
}

static inline int32 CompareExchange(volatile int32* Dest, int32 Exchange, int32 Comparand)
{
#if UT_WINDOWS
	return (int32)::_InterlockedCompareExchange((long*)Dest, (long)Exchange, (long)Comparand);
#elif UT_UNIX
	return __sync_val_compare_and_swap(Dest, Comparand, Exchange);
#else
	#error ut::atomics::interlocked::CompareExchange() is not implemented
#endif
}

static inline int64 CompareExchange(volatile int64* Dest, int64 Exchange, int64 Comparand)
{
#if UT_WINDOWS
	return (int64)::_InterlockedCompareExchange64(Dest, Exchange, Comparand);
#elif UT_UNIX
	return __sync_val_compare_and_swap(Dest, Comparand, Exchange);
#else
	#error ut::atomics::interlocked::CompareExchange() is not implemented
#endif
}

static inline int32 Read(volatile const int32* Src)
{
#if UT_WINDOWS
	return CompareExchange((int32*)Src, 0, 0);
#elif UT_UNIX
	int32 Result;
	__atomic_load((volatile int32*)Src, &Result, __ATOMIC_SEQ_CST);
	return Result;
#else
	#error ut::atomics::interlocked::Read() is not implemented
#endif
}

static inline int64 Read(volatile const int64* Src)
{
#if UT_WINDOWS
	return CompareExchange((int64*)Src, 0, 0);
#elif UT_UNIX
	int64 Result;
	__atomic_load((volatile int64*)Src, &Result, __ATOMIC_SEQ_CST);
	return Result;
#else
	#error ut::atomics::interlocked::Read() is not implemented
#endif
}

static inline void Store(volatile int32* Src, int32 Val)
{
#if UT_WINDOWS
	Exchange(Src, Val);
#elif UT_UNIX
	__atomic_store((volatile int32*)Src, &Val, __ATOMIC_SEQ_CST);
#else
	#error ut::atomics::interlocked::Store() is not implemented
#endif
}

static inline void Store(volatile int64* Src, int64 Val)
{
#if UT_WINDOWS
	Exchange(Src, Val);
#elif UT_UNIX
	__atomic_store((volatile int64*)Src, &Val, __ATOMIC_SEQ_CST);
#else
	#error ut::atomics::interlocked::Store() is not implemented
#endif
}

static inline void* CompareExchangePointer(void** Dest, void* Exchange, void* Comparand)
{
#if UT_WINDOWS
	#if UT_PLATFORM_64BITS
		return (void*)::_InterlockedCompareExchange64((int64*)Dest, (int64)Exchange, (int64)Comparand);
	#else
		return (void*)::_InterlockedCompareExchange((long*)Dest, (long)Exchange, (long)Comparand);
	#endif
#elif UT_UNIX
	return __sync_val_compare_and_swap(Dest, Comparand, Exchange);
#else
	#error ut::atomics::interlocked::CompareExchangePointer() is not implemented
#endif
}

//----------------------------------------------------------------------------//
END_NAMESPACE(interlocked)
END_NAMESPACE(atomics)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//