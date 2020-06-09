//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Mutex class is a synchronization primitive that can be used to protect
// shared data from being simultaneously accessed by multiple threads.
class Mutex : public NonCopyable
{
	friend class ConditionVariable;
public:
	// Default constructor, platform-specific object is constructed here
	Mutex();

	// Move constructor
	Mutex(Mutex&& other) noexcept;

	// Move operator
	Mutex& operator = (Mutex&& other) noexcept;

	// Destructor, platform-specific object is destructed here
	~Mutex();

	// Locks the mutex. If another thread has already locked the mutex,
	// a call to lock will block execution until the lock is acquired
	void Lock();

	// Unlocks the mutex. The mutex must be locked by the current
	// thread of execution, otherwise, the behavior is undefined
	void Unlock();

	// Just calls Lock() and then Unlock()
	void Sync();

private:
	// Platform-specific mutex type
#if UT_WINDOWS
	typedef CRITICAL_SECTION PlatformCriticalSection;
#elif UT_UNIX
	typedef pthread_mutex_t PlatformCriticalSection;
#else
#error ut::Mutex::PlatformMutexType is not implemented
#endif

	// Creates platform-specific 'mutex' object
	Optional<Error> Create();

	// Destroys platform-specific 'mutex' object
	void Destroy();

	// platform-specific critical section object
	ut::Optional<PlatformCriticalSection> platform_cs;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//