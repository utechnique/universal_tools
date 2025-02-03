//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_endianness.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(time)
//----------------------------------------------------------------------------//
// Constructor.
Counter::Counter() : active(false), suspended(0)
{
#if UT_WINDOWS
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
	{
		throw Error(error::not_supported, "QueryPerformanceFrequency");
	}
	frequency = li.QuadPart;
#endif
}

// Starts counting time.
void Counter::Start()
{
	start = GetCurrent();

	// resume case
	if (suspended != 0)
	{
		start -= suspended;
		suspended = 0;
	}

	active = true;
}

// Suspends to count time.
void Counter::Pause()
{
	UT_ASSERT(active);
	active = false;
	suspended = GetCurrent() - start;
}

// Stops and resets the counter.
void Counter::Stop()
{
	start = 0;
	suspended = 0;
	active = false;
}

// Returns current global time in nanoseconds.
ut::int64 Counter::GetCurrent()
{
#if UT_WINDOWS
	LARGE_INTEGER li;
	if (!QueryPerformanceCounter(&li))
	{
		throw Error(error::not_supported, "QueryPerformanceCounter");
	}
	return li.QuadPart;
#elif UT_LINUX
	struct timespec tp;
	int result = clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	if (result != 0)
	{
		throw Error(ConvertErrno(result), "clock_gettime");
	}

	ut::int64 sec_to_ns = Convert<Unit::second, Unit::nanosecond, ut::int64>(tp.tv_sec);
	return sec_to_ns + static_cast<ut::int64>(tp.tv_nsec);
#else
#error ut::TickCounter::GetCurrent() is not implemented
#endif
}

//----------------------------------------------------------------------------//
END_NAMESPACE(time)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//