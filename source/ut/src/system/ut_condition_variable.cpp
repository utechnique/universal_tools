//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_condition_variable.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Default constructor, platform-specific object is constructed here
ConditionVariable::ConditionVariable()
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Copy constructor, creates new synchronization object, nothing is really copied
ConditionVariable::ConditionVariable(const ConditionVariable& copy)
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}
//----------------------------------------------------------------------------->
// Assignment operator, creates new synchronization object,
// nothing is really copied, old object is deleted
ConditionVariable& ConditionVariable::operator = (const ConditionVariable& copy)
{
	Destroy();
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
	return *this;
}

//----------------------------------------------------------------------------->
// Destructor, platform-specific object is destructed here
ConditionVariable::~ConditionVariable()
{
	Destroy();
}

//----------------------------------------------------------------------------->
// Wait causes the current thread to block until the condition
// variable is notified or a spurious wakeup occurs.
//    @param lock - reference to the scope-lock object, which
//                  must be locked by the current thread.
void ConditionVariable::Wait(ScopeLock& lock)
{
#if UT_WINDOWS
	SleepConditionVariableCS(&cv, &lock.mutex.cs, INFINITE);
#elif UT_UNIX
	pthread_cond_wait(&cv, &lock.mutex.mutex);
#else
#error ut::ConditionVariable::Wait() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// If any threads are waiting on *this, calling WakeOne
// unblocks one of the waiting threads.
void ConditionVariable::WakeOne()
{
#if UT_WINDOWS
	WakeConditionVariable(&cv);
#elif UT_UNIX
	pthread_cond_signal(&cv);
#else
#error ut::ConditionVariable::WakeOne() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Unblocks all threads currently waiting for *this.
void ConditionVariable::WakeAll()
{
#if UT_WINDOWS
	WakeAllConditionVariable(&cv);
#elif UT_UNIX
	pthread_cond_broadcast(&cv);
#else
#error ut::ConditionVariable::WakeAll() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Creates platform-specific 'mutex' object
inline Optional<Error> ConditionVariable::Create()
{
#if UT_WINDOWS
	InitializeConditionVariable(&cv);
#elif UT_UNIX
	int result = pthread_cond_init(&cv, NULL);
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}
#else
#error ut::ConditionVariable::Create() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Destroys platform-specific 'mutex' object
inline void ConditionVariable::Destroy()
{
#if UT_WINDOWS
	// nothing to do
#elif UT_UNIX
	pthread_cond_destroy(&cv);
#else
#error ut::ConditionVariable::Destroy() is not implemented
#endif
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//