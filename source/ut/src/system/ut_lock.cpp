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
RWLock::RWLock() : data(PlatformData())
{
#if UT_WINDOWS
	data->readers_count = 0;
	data->writer_event = nullptr;
	data->no_readers_event = nullptr;
#elif UT_UNIX
	data->readers = 0;
	data->writers = 0;
	data->read_waiters = 0;
	data->write_waiters = 0;
#else
#error ut::RWLock::RWLock() is not implemented
#endif

	Optional<Error> creation_error = Create();
	if (creation_error)
	{
		throw creation_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Move constructor
RWLock::RWLock(RWLock&& other) noexcept : data(Move(other.data))
{}

//----------------------------------------------------------------------------->
// Move operator
RWLock& RWLock::operator = (RWLock&& other) noexcept
{
	Destroy();
	data = Move(other.data);
	return *this;
}

//----------------------------------------------------------------------------->
// Destructor, platform-specific synchronization primitives are destructed here
RWLock::~RWLock(void)
{
	Destroy();
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
	UT_ASSERT(data);
	if (access == access_write)
	{
		#if UT_WINDOWS
			EnterCriticalSection(&data->writer_lock);
			WaitForSingleObject(data->writer_event, INFINITE);
			ResetEvent(data->writer_event);
			WaitForSingleObject(data->no_readers_event, INFINITE);
			LeaveCriticalSection(&data->writer_lock);
		#elif UT_UNIX
			pthread_mutex_lock(&data->lock);
			if (data->readers || data->writers)
			{
				data->write_waiters++;
				do pthread_cond_wait(&data->write, &data->lock);
				while (data->readers || data->writers);
				data->write_waiters--;
			}
			data->writers = 1;
			pthread_mutex_unlock(&data->lock);
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
				WaitForSingleObject(data->writer_event, INFINITE);
				IncrementReaderCount();
				if (WaitForSingleObject(data->writer_event, 0) != WAIT_OBJECT_0)
				{
					DecrementReaderCount();
				}
				else
				{
					keep_loop = false;
				}
			}
		#elif UT_UNIX
			pthread_mutex_lock(&data->lock);
			if (data->writers || data->write_waiters)
			{
				data->read_waiters++;
				do pthread_cond_wait(&data->read, &data->lock);
				while (data->writers || data->write_waiters);
				data->read_waiters--;
			}
			data->readers++;
			pthread_mutex_unlock(&data->lock);
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
	UT_ASSERT(data);
	if (access == access_write)
	{
		#if UT_WINDOWS
			SetEvent(data->writer_event);
		#elif UT_UNIX
			pthread_mutex_lock(&data->lock);
			data->writers = 0;
			if (data->write_waiters)
			{
				pthread_cond_signal(&data->write);
			}
			else if (data->read_waiters)
			{
				pthread_cond_broadcast(&data->read);
			}
			pthread_mutex_unlock(&data->lock);
		#else
			#error ut::RWLock::Unlock() is not implemented
		#endif
	}
	else if (access == access_read)
	{
		#if UT_WINDOWS
			DecrementReaderCount();
		#elif UT_UNIX
			pthread_mutex_lock(&data->lock);
			data->readers--;
			if (data->write_waiters)
			{
				pthread_cond_signal(&data->write);
			}
			pthread_mutex_unlock(&data->lock);
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
	data->writer_event = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (data->writer_event == NULL)
	{
		return Error(ConvertWinSysErr(GetLastError()));
	}

	data->no_readers_event = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (data->no_readers_event == NULL)
	{
		return Error(ConvertWinSysErr(GetLastError()));
	}

	InitializeCriticalSection(&data->writer_lock);
	InitializeCriticalSection(&data->reader_lock);
#elif UT_UNIX
	int result = pthread_mutex_init(&data->lock, nullptr);
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}

	result = pthread_cond_init(&data->read, nullptr);
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}

	result = pthread_cond_init(&data->write, nullptr);
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
// Destroys platform-specific data
void RWLock::Destroy()
{
	PlatformData& platform_data = data.Get();
#if UT_WINDOWS
	DeleteCriticalSection(&data->writer_lock);
	DeleteCriticalSection(&data->reader_lock);
	CloseHandle(data->writer_event);
	CloseHandle(data->no_readers_event);
#elif UT_UNIX
	pthread_mutex_destroy(&data->lock);
	pthread_cond_destroy(&data->read);
	pthread_cond_destroy(&data->write);
#else
#error ut::RWLock::Destroy() is not implemented
#endif
}

//----------------------------------------------------------------------------->
// Windows-specific function, increases number of readers by one
#if UT_WINDOWS
void RWLock::IncrementReaderCount()
{
	EnterCriticalSection(&data->reader_lock);
	data->readers_count++;
	ResetEvent(data->no_readers_event);
	LeaveCriticalSection(&data->reader_lock);
}
#endif // UT_WINDOWS

//----------------------------------------------------------------------------->
// Windows-specific function, decreases number of readers by one
#if UT_WINDOWS
void RWLock::DecrementReaderCount()
{
	EnterCriticalSection(&data->reader_lock);
	data->readers_count--;
	if (data->readers_count <= 0)
	{
		SetEvent(data->no_readers_event);
	}
	LeaveCriticalSection(&data->reader_lock);
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