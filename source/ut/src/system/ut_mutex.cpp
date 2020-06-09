//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_mutex.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Default constructor, platform-specific object is constructed here
Mutex::Mutex() : platform_cs(PlatformCriticalSection())
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Move constructor
Mutex::Mutex(Mutex&& other) noexcept : platform_cs(Move(other.platform_cs))
{}

//----------------------------------------------------------------------------->
// Move operator
Mutex& Mutex::operator = (Mutex&& other) noexcept
{
	Destroy();
	platform_cs = Move(other.platform_cs);
	return *this;
}

//----------------------------------------------------------------------------->
// Destructor, platform-specific object is destructed here
Mutex::~Mutex()
{
	Destroy();
}

//----------------------------------------------------------------------------->
// Locks the mutex. If another thread has already locked the mutex,
// a call to lock will block execution until the lock is acquired
void Mutex::Lock()
{
#if UT_WINDOWS
	EnterCriticalSection(&platform_cs.Get());
#elif UT_UNIX
	pthread_mutex_lock(&platform_cs.Get());
#else
	#error ut::Mutex::Lock() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Unlocks the mutex. The mutex must be locked by the current
// thread of execution, otherwise, the behavior is undefined
void Mutex::Unlock()
{
#if UT_WINDOWS
	LeaveCriticalSection(&platform_cs.Get());
#elif UT_UNIX
	pthread_mutex_unlock(&platform_cs.Get());
#else
	#error ut::Mutex::Unlock() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Just calls Lock() and then Unlock()
void Mutex::Sync()
{
	Lock();
	Unlock();
}

//----------------------------------------------------------------------------->
// Creates platform-specific 'mutex' object
Optional<Error> Mutex::Create()
{
#if UT_WINDOWS
	InitializeCriticalSection(&platform_cs.Get());
#elif UT_UNIX
	int result = pthread_mutex_init(&platform_cs.Get(), NULL);
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}
#else
	#error ut::Mutex::Create() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Destroys platform-specific 'mutex' object
void Mutex::Destroy()
{
	if (platform_cs)
	{
#if UT_WINDOWS
		DeleteCriticalSection(&platform_cs.Get());
#elif UT_UNIX
		pthread_mutex_destroy(&platform_cs.Get());
#else
#error ut::Mutex::Destroy() is not implemented
#endif
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//