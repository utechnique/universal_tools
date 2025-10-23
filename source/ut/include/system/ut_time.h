//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_array.h"
#include "templates/ut_enable_if.h"
#include "math/ut_average.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(time)
//----------------------------------------------------------------------------//
// Measurement units.
enum class Unit
{
	nanosecond = 0,
	microsecond = 1,
	millisecond = 2,
	second = 3,
	minute = 4,
	hour = 5,
	day = 6
};

// Epoch dates.
enum class Epoch
{
	unix, // January 1, 1970 UTC
	windows // January 1, 1601 UTC
};

// ut::time::Multiplier describes the relationship
// between different time units.
template<Unit unit> struct Multiplier;
template<> struct Multiplier<Unit::microsecond> { static constexpr ut::uint32 value = 1000; };
template<> struct Multiplier<Unit::millisecond> { static constexpr ut::uint32 value = 1000; };
template<> struct Multiplier<Unit::second>      { static constexpr ut::uint32 value = 1000; };
template<> struct Multiplier<Unit::minute>      { static constexpr ut::uint32 value = 60; };
template<> struct Multiplier<Unit::hour>        { static constexpr ut::uint32 value = 60; };
template<> struct Multiplier<Unit::day>         { static constexpr ut::uint32 value = 24; };

// ut::time::UnitIterator helps transform time units.
template<Unit start_unit, ut::int32 walk>
struct UnitIterator
{
	// Boolean whether iterating forward or backward.
	static constexpr bool forward = walk > 0;

	// Offset to the next unit type.
	static constexpr ut::int32 step = forward ? 1 : -1;

	// Next unit type.
	static constexpr Unit next_unit = static_cast<Unit>(static_cast<ut::int32>(start_unit) + step);

	// Next iterator type.
	typedef UnitIterator<next_unit, walk - step> NextIterator;


	// Returns multiplier that is accumulated recursively while iterating forward.
	template<typename ValueType>
	static constexpr typename EnableIf<forward, ValueType>::Type Accumulate()
	{
		return Multiplier<next_unit>::value * NextIterator::template Accumulate<ValueType>();
	}

	// Returns multiplier that is accumulated recursively while iterating backward.
	template<typename ValueType>
	static constexpr typename EnableIf<!forward, ValueType>::Type Accumulate()
	{
		return Multiplier<start_unit>::value * NextIterator::template Accumulate<ValueType>();
	}

	// Returns converted value.
	template<typename ValueType>
	static constexpr typename EnableIf<forward, ValueType>::Type Multiply(ValueType value)
	{
		return value / Accumulate<ValueType>();
	}

	// Returns converted value.
	template<typename ValueType>
	static constexpr typename EnableIf<!forward, ValueType>::Type Multiply(ValueType value)
	{
		return value * Accumulate<ValueType>();
	}
};

// Specialized version for the final unit type in a sequence.
template<Unit start_unit> struct UnitIterator<start_unit, 0>
{
	template<typename ValueType> static constexpr ValueType Accumulate()
	{
		return static_cast<ValueType>(1);
	}

	template<typename ValueType> static constexpr ValueType Multiply(ValueType value)
	{
		return value;
	}
};

// Converts provided value to another time unit.
template<Unit from_unit, Unit to_unit, typename ValueType = double>
constexpr ValueType Convert(ValueType value)
{
	return UnitIterator<from_unit, static_cast<ut::int32>(to_unit) - static_cast<ut::int32>(from_unit)>::template Multiply<ValueType>(value);
}

// Precise time counter. Use it only for short periods of time, intervals
// more than a day may produce unreliable results.
class Counter
{
public:
	// Constructor.
	Counter();

	// Starts counting time.
	void Start();

	// Suspends to count time.
	void Pause();

	// Stops and resets the counter.
	void Stop();

	// Returns time passed from the last time::Counter::Start function call.
	// Resolution: 1 nanosecond.
	template<Unit time_unit = Unit::millisecond, typename ValueType = double>
	ValueType GetTime() const // nanosec
	{
		ut::int64 time_elapsed_ns = suspended != 0 ? suspended : GetCurrent() - start;
#if UT_WINDOWS
		double sec = static_cast<double>(time_elapsed_ns) / frequency;
		return static_cast<ValueType>(Convert<Unit::second, time_unit, double>(sec));
#endif
		return Convert<Unit::nanosecond, time_unit, ValueType>(static_cast<ValueType>(time_elapsed_ns));
	}

private:
	// Returns current global time in nanoseconds.
	static ut::int64 GetCurrent();

	// indicates if counter is active
	bool active;

	// global time when counter started (nanoseconds)
	ut::int64 start;

	// time spent in suspended (paused) state (nanoseconds)
	ut::int64 suspended;

	// frequency of windows performance counter (per second)
#if UT_WINDOWS
	ut::int64 frequency;
#endif
};

// ut::time::PerformanceCounter is a template class to measure performance of
// the recurrent actions more smoothly than ut::time::Counter. Smoothness is
// achieved by caching measurements and calculating the average value. Unlike
// ut::time::Counter, ut::time::PerformanceCounter::GetTime function must be
// called after stopping the counter by ut::time::PerformanceCounter::Stop
// function call.
template<int iteration_count>
class PerformanceCounter : public time::Counter
{
public:
	// Constructor.
	PerformanceCounter() : iterator(0), iterations{0}
	{}

	// Starts counting time.
	void Start()
	{
		time::Counter::Start();
		if (++iterator >= iteration_count)
		{
			iterator = 0;
		}
	}

	// Stops and resets the counter.
	void Stop()
	{
		iterations[iterator] = Counter::GetTime<Unit::nanosecond, ut::uint64>();
		Counter::Stop();
	}

	// Returns the average time of all measurements.
	template<Unit time_unit = Unit::millisecond, typename ValueType = double>
	ValueType GetTime() const
	{
		return Convert<Unit::nanosecond, time_unit, ValueType>(static_cast<ValueType>(Average(iterations)));
	}

private:
	int iterator;
	ut::uint64 iterations[iteration_count];
};

// Returns the time since the desired epoch.
// Resolution: 1 microsecond.
template<time::Unit time_unit = Unit::millisecond, Epoch epoch = Epoch::unix>
ut::uint64 GetTime()
{
#if UT_WINDOWS
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);

	LARGE_INTEGER li;
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	if (epoch == Epoch::unix)
	{
		li.QuadPart -= 0x019DB1DED53E8000;
	}

	li.QuadPart /= 10; // converting from 100ns to microseconds

	return Convert<Unit::microsecond, time_unit, ut::uint64>(static_cast<ut::uint64>(li.QuadPart));
#elif UT_UNIX
	struct timeval tm;
	gettimeofday(&tm, NULL);

	ut::uint64 tm_microsec = static_cast<ut::uint64>(tm.tv_sec * 1000000 + tm.tv_usec);

	if (epoch == Epoch::windows)
	{
		tm_microsec += 11644473600000000LL;
	}

	return Convert<Unit::microsecond, time_unit, ut::uint64>(static_cast<ut::uint64>(tm_microsec));
#else
#error ut::time::GetTime() is not implemented
#endif
}

//----------------------------------------------------------------------------//
END_NAMESPACE(time)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//