//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_mutex.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Default constructor, platform-specific object is constructed here
Mutex::Mutex()
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}
//----------------------------------------------------------------------------->
// Copy constructor, creates new 'mutex' object
Mutex::Mutex(const Mutex& copy)
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}
//----------------------------------------------------------------------------->
// Assignment operator, creates new 'mutex' object,
// nothing is really copied, old @mutex object is deleted
Mutex& Mutex::operator = (const Mutex& copy)
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
	WaitForSingleObject(mutex, INFINITE);
#elif UT_UNIX
	pthread_mutex_lock(&mutex);
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
	ReleaseMutex(mutex);
#elif UT_UNIX
	pthread_mutex_unlock(&mutex);
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
inline Optional<Error> Mutex::Create()
{
#if UT_WINDOWS
	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		return Error(ConvertWinSysErr(GetLastError()));
	}
#elif UT_UNIX
	int result = pthread_mutex_init(&mutex, NULL);
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
inline void Mutex::Destroy()
{
#if UT_WINDOWS
	CloseHandle(mutex);
#elif UT_UNIX
	pthread_mutex_destroy(&mutex);
#else
	#error ut::Mutex::Destroy() is not implemented
#endif
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//