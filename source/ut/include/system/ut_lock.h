//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "system/ut_mutex.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Access enumeration describes possible access
// to objects that are shared between different threads
//    @access_full - full access
//    @access_read - managed object can only be read, writing is forbidden
//    @access_write - writing allowed to the managed object
enum Access
{
	access_full,
	access_read,
	access_write
};

//----------------------------------------------------------------------------//
// ut::ScopeLock is a mutex wrapper that provides a convenient RAII-style
// mechanism for owning a mutex for the duration of a scoped block
class ScopeLock : public NonCopyable
{
	friend class ConditionVariable;
public:
	// Constructor, @mutex is locked here
	//    @in_mutex - mutex to lock
	ScopeLock(Mutex& in_mutex);

	// Destructor, @mutex is unlocked here
	~ScopeLock();

private:
	// Managed mutex object
	Mutex& mutex;
};

//----------------------------------------------------------------------------//
// ut::RWLock is a synchronization primitive, providing 'read' and 'write'
// access to the managed object. Rule is simple: one writes - everybody waits,
// nobody writes - everybody can read simultaneously.
class RWLock : public NonCopyable
{
public:
	// Constructor, critical sections and condition variables are initialize here
	// Windows realization uses two critical sections and two conditional variables
	// Linux realization uses two conditional variables and a mutex
	RWLock();

	// Copy constructor, creates new 'RWLock' object, nothing is really copied
	RWLock(const RWLock& copy);

	// Destructor, platform-specific synchronization primitives are destructed here
	~RWLock();

	// Locks section with 'access_write' access
	void Lock();

	// Unlocks section with 'access_write' access
	void Unlock();

	// Locks section with desired access
	//    @param access - access to lock section with
	//                    see ut::Access for details
	void Lock(Access access);

	// Unlocks section with desired access
	//    @param access - access to unlock section with
	//                    see ut::Access for details
	void Unlock(Access access);

private:
	// Creates synchronization objects
	Optional<Error> Create();

#if UT_WINDOWS
	// Windows-specific function, increases number of readers by one
	void IncrementReaderCount();

	// Windows-specific function, decreases number of readers by one
	void DecrementReaderCount();

	// @writer_event is used to wait writer before locking
	HANDLE writer_event;

	// @no_readers_event is set when @readers_count is null
	// this event signals, that writer can write
	HANDLE no_readers_event;

	// @readers_count is used for counting readers
	int readers_count;

	// @writer_lock is a windows crytical section object for writers
	CRITICAL_SECTION writer_lock;

	// @writer_lock is a windows crytical section object for readers
	CRITICAL_SECTION reader_lock;
#elif UT_UNIX
	pthread_mutex_t lock;
	pthread_cond_t read;
	pthread_cond_t write;
	uint32 readers;
	uint32 writers;
	uint32 read_waiters;
	uint32 write_waiters;
#else
	#error ut::RWLock is not implemented
#endif
};

//----------------------------------------------------------------------------//
// ut::ScopeRWLock is a ut::RWLock wrapper that provides a convenient RAII-style
// mechanism for owning one ut::RWLock object for the duration of a scoped block
class ScopeRWLock : public NonCopyable
{
public:
	// Constructor, @lock is locked here
	//    @param in_lock - object to lock
	//    @param in_access - object will be locked and (then)
	//                       unlocked with this flag
	ScopeRWLock(RWLock& in_lock, Access in_access = access_write);

	// Constructor, @lock is unlocked here with @access key
	~ScopeRWLock();

	// Unlocks @lock with @access key, call it if you
	// want to release @mutex before the end of scope
	void Reset();

private:
	// @lock is a pointer to the ut::RWLock
	// object to lock inside the current scope
	RWLock* lock;

	// @lock is locked and unlocked with this flag
	Access access;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//