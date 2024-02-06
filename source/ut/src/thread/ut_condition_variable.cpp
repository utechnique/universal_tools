//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "thread/ut_condition_variable.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Default constructor, platform-specific object is constructed here
ConditionVariable::ConditionVariable() : platform_cv(PlatformCvType())
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Move constructor
ConditionVariable::ConditionVariable(ConditionVariable&& other) noexcept : platform_cv(Move(other.platform_cv))
{}

//----------------------------------------------------------------------------->
// Move operator
ConditionVariable& ConditionVariable::operator = (ConditionVariable&& other) noexcept
{
	Destroy();
	platform_cv = Move(other.platform_cv);
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
	SleepConditionVariableCS(&platform_cv.Get(), &lock.mutex.platform_cs.Get(), INFINITE);
#elif UT_UNIX
	pthread_cond_wait(&platform_cv.Get(), &lock.mutex.platform_cs.Get());
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
	WakeConditionVariable(&platform_cv.Get());
#elif UT_UNIX
	pthread_cond_signal(&platform_cv.Get());
#else
#error ut::ConditionVariable::WakeOne() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Unblocks all threads currently waiting for *this.
void ConditionVariable::WakeAll()
{
#if UT_WINDOWS
	WakeAllConditionVariable(&platform_cv.Get());
#elif UT_UNIX
	pthread_cond_broadcast(&platform_cv.Get());
#else
#error ut::ConditionVariable::WakeAll() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Creates platform-specific 'mutex' object
Optional<Error> ConditionVariable::Create()
{
#if UT_WINDOWS
	InitializeConditionVariable(&platform_cv.Get());
#elif UT_UNIX
	int result = pthread_cond_init(&platform_cv.Get(), NULL);
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
void ConditionVariable::Destroy()
{
	if (platform_cv)
	{
#if UT_WINDOWS
		// nothing to do
#elif UT_UNIX
		pthread_cond_destroy(&platform_cv.Get());
#else
#error ut::ConditionVariable::Destroy() is not implemented
#endif
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//