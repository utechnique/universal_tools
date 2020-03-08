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
class Mutex
{
	friend class ConditionVariable;
public:
	// Default constructor, platform-specific object is constructed here
	Mutex();

	// Copy constructor, creates new 'mutex' object, nothing is really copied
	Mutex(const Mutex& copy);

	// Assignment operator, creates new 'mutex' object,
	// nothing is really copied, old @mutex object is deleted
	Mutex& operator = (const Mutex &copy);

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
	// Creates platform-specific 'mutex' object, should be inline
	Optional<Error> Create();

	// Destroys platform-specific 'mutex' object, should be inline
	void Destroy();

	// variable named 'mutex' is platform-specific mutex object
#if UT_WINDOWS
	CRITICAL_SECTION cs;
#elif UT_UNIX
	pthread_mutex_t mutex; // using pthread library to obtain linux mutex
#else
	#error ut::Mutex is not implemented
#endif
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//