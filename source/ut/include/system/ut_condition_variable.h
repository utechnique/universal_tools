//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
#include "system/ut_lock.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::ConditionVariable is an object able to block the calling thread
// until notified to resume.
class ConditionVariable : NonCopyable
{
public:
	// Default constructor, platform-specific object is constructed here
	ConditionVariable();

	// Copy constructor, creates new synchronization object, nothing is really copied
	ConditionVariable(const ConditionVariable& copy);

	// Assignment operator, creates new synchronization object,
	// nothing is really copied, old object is deleted
	ConditionVariable& operator = (const ConditionVariable &copy);

	// Destructor, platform-specific object is destructed here
	~ConditionVariable();

	// Looping Wait() function until some predicate is satisfied.
	//    @param lock - reference to the scope-lock object, which
	//                  must be locked by the current thread.
	//    @param pred - predicate which returns false if the waiting should be
	//                  continued; the signature of the predicate function should
	//                  be equivalent to the following: bool pred();
	template<typename Predicate>
	void Wait(ScopeLock& lock, Predicate pred)
	{
		while (!pred())
		{
			Wait(lock);
		}
	}

	// Wait causes the current thread to block until the condition
	// variable is notified or a spurious wakeup occurs.
	//    @param lock - reference to the scope-lock object, which
	//                  must be locked by the current thread.
	void Wait(ScopeLock& lock);

	// If any threads are waiting on *this, calling WakeOne
	// unblocks one of the waiting threads.
	void WakeOne();

	// Unblocks all threads currently waiting for *this.
	void WakeAll();

private:
	// Creates platform-specific synchronization object
	Optional<Error> Create();

	// Destroys platform-specific synchronization object
	void Destroy();

	// variable named 'cv' is platform-specific synchronization object
#if UT_WINDOWS
	CONDITION_VARIABLE cv;
#elif UT_UNIX
	pthread_cond_t cv;
#else
#error ut::ConditionVariable is not implemented
#endif
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//