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

static inline int32 Increment(volatile int32* val)
{
#if UT_WINDOWS
	return (int32)_InterlockedIncrement((long*)val);
#elif UT_UNIX
	return __sync_fetch_and_add(val, 1) + 1;
#else
#error ut::atomics::interlocked::Increment() is not implemented
#endif
}

static inline int64 Increment(volatile int64* val)
{
#if UT_WINDOWS
#if UT_PLATFORM_64BITS
	return (int64)::_InterlockedIncrement64((long long*)val);
#else
	// No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
	for (;;)
	{
		int64 old_val = *val;
		if (_InterlockedCompareExchange64(val, old_val + 1, old_val) == old_val)
		{
			return old_val + 1;
		}
	}
#endif
#elif UT_UNIX
	return __sync_fetch_and_add(val, 1) + 1;
#else
#error ut::atomics::interlocked::Increment() is not implemented
#endif
}

static inline int32 Decrement(volatile int32* val)
{
#if UT_WINDOWS
	return (int32)::_InterlockedDecrement((long*)val);
#elif UT_UNIX
	return __sync_fetch_and_sub(val, 1) - 1;
#else
#error ut::atomics::interlocked::Decrement() is not implemented
#endif
}

static inline int64 Decrement(volatile int64* val)
{
#if UT_WINDOWS
#if UT_PLATFORM_64BITS
	return (int64)::_InterlockedDecrement64((long long*)val);
#else
	// No explicit instruction for 64-bit atomic decrement on 32-bit processors; has to be implemented in terms of CMPXCHG8B
	for (;;)
	{
		int64 old_val = *val;
		if (_InterlockedCompareExchange64(val, old_val - 1, old_val) == old_val)
		{
			return old_val - 1;
		}
	}
#endif
#elif UT_UNIX
	return __sync_fetch_and_sub(val, 1) - 1;
#else
#error ut::atomics::interlocked::Decrement() is not implemented
#endif
}

static inline int32 Add(volatile int32* val, int32 amount)
{
#if UT_WINDOWS
	return (int32)::_InterlockedExchangeAdd((long*)val, (long)amount);
#elif UT_UNIX
	return __sync_fetch_and_add(val, amount);
#else
#error ut::atomics::interlocked::Add() is not implemented
#endif
}

static inline int64 Add(volatile int64* val, int64 amount)
{
#if UT_WINDOWS
#if UT_PLATFORM_64BITS
	return (int64)::_InterlockedExchangeAdd64((int64*)val, (int64)amount);
#else
	// No explicit instruction for 64-bit atomic add on 32-bit processors; has to be implemented in terms of CMPXCHG8B
	for (;;)
	{
		int64 old_val = *val;
		if (_InterlockedCompareExchange64(val, old_val + amount, old_val) == old_val)
		{
			return old_val + amount;
		}
	}
#endif
#elif UT_UNIX
	return __sync_fetch_and_add(val, amount);
#else
#error ut::atomics::interlocked::Add() is not implemented
#endif
}

static inline int32 Exchange(volatile int32* val, int32 exchange)
{
#if UT_WINDOWS
	return (int32)::_InterlockedExchange((long*)val, (long)exchange);
#elif UT_UNIX
	return __sync_lock_test_and_set(val, exchange);
#else
#error ut::atomics::interlocked::Exchange() is not implemented
#endif
}

static inline int64 Exchange(volatile int64* val, int64 exchange)
{
#if UT_WINDOWS
#if UT_PLATFORM_64BITS
	return (int64)::_InterlockedExchange64((long long*)val, (long long)exchange);
#else
	// No explicit instruction for 64-bit atomic exchange on 32-bit processors; has to be implemented in terms of CMPXCHG8B
	for (;;)
	{
		int64 old_val = *val;
		if (_InterlockedCompareExchange64(val, exchange, old_val) == old_val)
		{
			return old_val;
		}
	}
#endif
#elif UT_UNIX
	return __sync_lock_test_and_set(val, exchange);
#else
#error ut::atomics::interlocked::Exchange() is not implemented
#endif
}

static inline void* ExchangePtr(void** dst, void* exchange)
{
#if UT_WINDOWS
#if UT_PLATFORM_64BITS
	return (void*)::_InterlockedExchange64((int64*)(dst), (int64)(exchange));
#else
	return (void*)::_InterlockedExchange((long*)(dst), (long)(exchange));
#endif
#elif UT_UNIX
	return __sync_lock_test_and_set(dst, exchange);
#else
#error ut::atomics::interlocked::ExchangePtr() is not implemented
#endif
}

static inline int32 CompareExchange(volatile int32* dst, int32 exchange, int32 comparand)
{
#if UT_WINDOWS
	return (int32)::_InterlockedCompareExchange((long*)dst, (long)exchange, (long)comparand);
#elif UT_UNIX
	return __sync_val_compare_and_swap(dst, comparand, exchange);
#else
#error ut::atomics::interlocked::CompareExchange() is not implemented
#endif
}

static inline int64 CompareExchange(volatile int64* dst, int64 exchange, int64 comparand)
{
#if UT_WINDOWS
	return (int64)::_InterlockedCompareExchange64(dst, exchange, comparand);
#elif UT_UNIX
	return __sync_val_compare_and_swap(dst, comparand, exchange);
#else
#error ut::atomics::interlocked::CompareExchange() is not implemented
#endif
}

static inline int32 Read(volatile const int32* src)
{
#if UT_WINDOWS
	return CompareExchange((int32*)src, 0, 0);
#elif UT_UNIX
	int32 result;
	__atomic_load((volatile int32*)src, &result, __ATOMIC_SEQ_CST);
	return result;
#else
#error ut::atomics::interlocked::Read() is not implemented
#endif
}

static inline int64 Read(volatile const int64* src)
{
#if UT_WINDOWS
	return CompareExchange((int64*)src, 0, 0);
#elif UT_UNIX
	int64 result;
	__atomic_load((volatile int64*)src, &result, __ATOMIC_SEQ_CST);
	return result;
#else
#error ut::atomics::interlocked::Read() is not implemented
#endif
}

static inline void Store(volatile int32* src, int32 val)
{
#if UT_WINDOWS
	Exchange(src, val);
#elif UT_UNIX
	__atomic_store((volatile int32*)src, &val, __ATOMIC_SEQ_CST);
#else
#error ut::atomics::interlocked::Store() is not implemented
#endif
}

static inline void Store(volatile int64* src, int64 val)
{
#if UT_WINDOWS
	Exchange(src, val);
#elif UT_UNIX
	__atomic_store((volatile int64*)src, &val, __ATOMIC_SEQ_CST);
#else
#error ut::atomics::interlocked::Store() is not implemented
#endif
}

static inline void* CompareExchangePointer(void** dst, void* exchange, void* comparand)
{
#if UT_WINDOWS
#if UT_PLATFORM_64BITS
	return (void*)::_InterlockedCompareExchange64((int64*)dst, (int64)exchange, (int64)comparand);
#else
	return (void*)::_InterlockedCompareExchange((long*)dst, (long)exchange, (long)comparand);
#endif
#elif UT_UNIX
	return __sync_val_compare_and_swap(dst, comparand, exchange);
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