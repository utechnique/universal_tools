//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_lock.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// class ut::ScopeLock                                                        //
//----------------------------------------------------------------------------//
// Constructor, @mutex is locked here
//    @in_mutex - mutex to lock
ScopeLock::ScopeLock(Mutex& in_mutex) : mutex(in_mutex)
{
	mutex.Lock();
}

//----------------------------------------------------------------------------->
// Destructor, @mutex is unlocked here
ScopeLock::~ScopeLock()
{
	mutex.Unlock();
}

//----------------------------------------------------------------------------//
// class ut::RWLock                                                           //
//----------------------------------------------------------------------------//
// Constructor, critical sections and condition variables are initialize here
// Windows realization uses two critical sections and two conditional variables
// Linux realization uses two conditional variables and a mutex
RWLock::RWLock()
#if UT_WINDOWS
	: readers_count(0), writer_event(nullptr), no_readers_event(nullptr)
#elif UT_UNIX
	: readers(0), writers(0), read_waiters(0), write_waiters(0)
#endif
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Copy constructor, creates new 'RWLock' object, nothing is really copied
RWLock::RWLock(const RWLock& copy)
#if UT_WINDOWS
	: readers_count(0), writer_event(nullptr), no_readers_event(nullptr)
#elif UT_UNIX
	: readers(0), writers(0), read_waiters(0), write_waiters(0)
#endif
{
	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Destructor, platform-specific synchronization primitives are destructed here
RWLock::~RWLock(void)
{
#if UT_WINDOWS
	DeleteCriticalSection(&writer_lock);
	DeleteCriticalSection(&reader_lock);
	CloseHandle(writer_event);
	CloseHandle(no_readers_event);
#elif UT_UNIX
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&read);
	pthread_cond_destroy(&write);
#else
	#error ut::RWLock destructor is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Locks section with 'access_write' access
void RWLock::Lock(void)
{
	Lock(access_write);
}

//----------------------------------------------------------------------------->
// Unlocks section with 'access_write' access
void RWLock::Unlock(void)
{
	Unlock(access_write);
}

//----------------------------------------------------------------------------->
// Locks section with desired access
//    @param access - access to lock section with
//                    see ut::Access for details
void RWLock::Lock(Access access)
{
	if (access == access_write)
	{
		#if UT_WINDOWS
			EnterCriticalSection(&writer_lock);
			WaitForSingleObject(writer_event, INFINITE);
			ResetEvent(writer_event);
			WaitForSingleObject(no_readers_event, INFINITE);
			LeaveCriticalSection(&writer_lock);
		#elif UT_UNIX
			pthread_mutex_lock(&lock);
			if (readers || writers)
			{
				write_waiters++;
				do pthread_cond_wait(&write, &lock);
				while (readers || writers);
				write_waiters--;
			}
			writers = 1;
			pthread_mutex_unlock(&lock);
		#else
			#error ut::RWLock::Lock() is not implemented
		#endif
	}
	else if (access == access_read)
	{
		#if UT_WINDOWS
			bool keep_loop = true;
			while (keep_loop)
			{
				WaitForSingleObject(writer_event, INFINITE);
				IncrementReaderCount();
				if (WaitForSingleObject(writer_event, 0) != WAIT_OBJECT_0)
				{
					DecrementReaderCount();
				}
				else
				{
					keep_loop = false;
				}
			}
		#elif UT_UNIX
			pthread_mutex_lock(&lock);
			if (writers || write_waiters)
			{
				read_waiters++;
				do pthread_cond_wait(&read, &lock);
				while (writers || write_waiters);
				read_waiters--;
			}
			readers++;
			pthread_mutex_unlock(&lock);
		#else
			#error ut::RWLock::Lock() is not implemented
		#endif
	}
	else
	{
		Lock();
	}
}

//----------------------------------------------------------------------------->
// Unlocks section with desired access
//    @param access - access to unlock section with
//                    see ut::Access for details
void RWLock::Unlock(Access access)
{
	if (access == access_write)
	{
		#if UT_WINDOWS
			SetEvent(writer_event);
		#elif UT_UNIX
			pthread_mutex_lock(&lock);
			writers = 0;
			if (write_waiters)
			{
				pthread_cond_signal(&write);
			}
			else if (read_waiters)
			{
				pthread_cond_broadcast(&read);
			}
			pthread_mutex_unlock(&lock);
		#else
			#error ut::RWLock::Unlock() is not implemented
		#endif
	}
	else if (access == access_read)
	{
		#if UT_WINDOWS
			DecrementReaderCount();
		#elif UT_UNIX
			pthread_mutex_lock(&lock);
			readers--;
			if (write_waiters)
			{
				pthread_cond_signal(&write);
			}
			pthread_mutex_unlock(&lock);
		#else
			#error ut::RWLock::Unlock() is not implemented
		#endif
	}
	else
	{
		Unlock();
	}
}

//----------------------------------------------------------------------------->
// Creates synchronization objects
inline Optional<Error> RWLock::Create()
{
#if UT_WINDOWS
	writer_event = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (writer_event == NULL)
	{
		return Error(ConvertWinSysErr(GetLastError()));
	}

	no_readers_event = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (no_readers_event == NULL)
	{
		return Error(ConvertWinSysErr(GetLastError()));
	}

	InitializeCriticalSection(&writer_lock);
	InitializeCriticalSection(&reader_lock);
#elif UT_UNIX
	int result = pthread_mutex_init(&lock, NULL);
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}

	result = pthread_cond_init(&read, NULL);
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}

	result = pthread_cond_init(&write, NULL);
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}
#else
	#error ut::Create() is not implemented
#endif
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Windows-specific function, increases number of readers by one
#if UT_WINDOWS
void RWLock::IncrementReaderCount()
{
	EnterCriticalSection(&reader_lock);
	readers_count++;
	ResetEvent(no_readers_event);
	LeaveCriticalSection(&reader_lock);
}
#endif // UT_WINDOWS

//----------------------------------------------------------------------------->
// Windows-specific function, decreases number of readers by one
#if UT_WINDOWS
void RWLock::DecrementReaderCount()
{
	EnterCriticalSection(&reader_lock);
	readers_count--;
	if (readers_count <= 0)
	{
		SetEvent(no_readers_event);
	}
	LeaveCriticalSection(&reader_lock);
}
#endif // UT_WINDOWS

//----------------------------------------------------------------------------//
// class ut::ScopeRWLock                                                      //
//----------------------------------------------------------------------------//
// Constructor, @lock is locked here
//    @param in_lock - object to lock
//    @param in_access - object will be locked and (then)
//                       unlocked with this flag
ScopeRWLock::ScopeRWLock(RWLock& in_lock, Access in_access) : lock(&in_lock), access(in_access)
{
	lock->Lock(access);
}

//----------------------------------------------------------------------------->
// Constructor, @lock is unlocked here with @access key
ScopeRWLock::~ScopeRWLock(void)
{
	Reset();
}

//----------------------------------------------------------------------------->
// Unlocks @lock with @access key, call it if you
// want to release @mutex before the end of scope
void ScopeRWLock::Reset(void)
{
	if (lock)
	{
		lock->Unlock(access);
		lock = nullptr;
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//