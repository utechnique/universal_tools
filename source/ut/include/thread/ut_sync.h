//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "thread/ut_mutex.h"
#include "thread/ut_lock.h"
#include "thread/ut_condition_variable.h"
#include "thread/ut_thread.h"
#include "pointers/ut_shared_ptr.h"
#include "pointers/ut_weak_ptr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Sync template class is a thread-safe container for @object.
// You can safely get the value by calling ut::Sync::Get(), and safely set
// the value by calling ut::Sync::Set() or assignment operator.
// You can also lock any code segment with ut::Sync::Lock(), and unlock it
// with ut::Sync::Unlock(), inside such a section you must use the reference
// received from calling ut::Sync::Lock(), but not ut::SyncRW::Get(),
// ut::SyncRW::Set() or assignment operator.
template<typename T>
class Synchronized : public NonCopyable
{
public:
	typedef RWLock::Access Access;

	// Default constructor
	Synchronized() {}

	// Move constructor
	Synchronized(Synchronized&& other) noexcept : object(Move(other.object))
	                                            , m(Move(other.m))
	{}

	// Constructor, @object is explicitly copied from @copy
	Synchronized(const T& copy) : object(copy) {}

	// Constructor, @object is explicitly copied from @copy
	Synchronized(T&& rval) : object(Move(rval)) {}

	// Move operator
	Synchronized& operator = (Synchronized&& other) noexcept
	{
		object = Move(other.object);
		m = Move(other.m);
		return *this;
	}

	// Assignment operator, @object is safely extracted from @copy
	Synchronized& operator = (const T& copy)
	{
		Set(copy);
		return *this;
	}


	// Getter function, safely returns a copy of managed object
	//    @return - copy(!) of the @object
	T Get()
	{
		ScopeLock scl(m);
		return object;
	}

	// Locks mutex @m, and returns @object reference
	T& Lock()
	{
		m.Lock();
		return object;
	}

	// Unlocks mutex @m
	void Unlock()
	{
		m.Unlock();
	}

	// Setter function, @object is safely extracted from @copy
	void Set(const T& copy)
	{
		ScopeLock scl(m);
		object = copy;
	}

	// Setter function, @object is moved from @rval
	void Set(T&& rval)
	{
		ScopeLock scl(m);
		object = Move(rval);
	}

private:
	// Managed object
	T object;

	// Mutex to provide thread-safe behaviour
	Mutex m;
};

//----------------------------------------------------------------------------//
// ut::SyncRW template class is a thread-safe container for @object that
// implements 'read-write lock' pattern.
// You can safely get the value by calling ut::SyncRW::Get(), and safely set
// the value by calling ut::SyncRW::Set() or assignment operator.
// You can also lock any code segment with ut::SyncRW::Lock(), and unlock it
// with ut::SyncRW::Unlock(), inside such a section you must use the reference
// received from calling ut::SyncRW::Lock(), but not ut::SyncRW::Get(),
// ut::SyncRW::Set() or assignment operator.
template<typename T>
class SyncRW
{
public:
	typedef RWLock::Access Access;

	// Default constructor
	SyncRW() {}

	// Copy constructor, @object is safely extracted from @copy
	SyncRW(SyncRW& copy) : object(copy.Get()) {}

	// Copy constructor, @object is explicitly copied from @copy
	SyncRW(const T& copy) : object(copy) {}

	// Move constructor, @object is explicitly copied from @copy
	SyncRW(T&& rval) noexcept : object(Move(rval)) {}

	// Assignment operator, @object is safely extracted from @copy
	SyncRW& operator = (SyncRW& copy)
	{
		Set(copy.Get());
		return *this;
	}

	// Assignment operator, @object is safely extracted from @copy
	SyncRW& operator = (const T& copy)
	{
		Set(copy);
		return *this;
	}

	// Getter function, safely returns a copy of managed object
	//    @return - copy(!) of the @object
	T Get()
	{
		ScopeRWLock scl(lock, Access::read);
		return object;
	}

	// Locks mutex @m, and returns @object reference
	T& Lock(Access access = Access::write)
	{
		lock.Lock(access);
		return object;
	}

	// Unlocks mutex @m
	void Unlock(Access access = Access::write)
	{
		lock.Unlock(access);
	}

	// Setter function, @object is safely extracted from @copy
	void Set(const T& copy)
	{
		ScopeRWLock scl(lock, Access::write);
		object = copy;
	}

	// Setter function, @object is moved from @rval
	void Set(T&& rval)
	{
		ScopeRWLock scl(lock, Access::write);
		object = Move(rval);
	}

private:
	// Managed object
	T object;

	// Read-Write lock to provide thread-safe behaviour
	RWLock lock;
};

//----------------------------------------------------------------------------//
// ut::ScopeSyncLock is a ut::Sync wrapper that provides a convenient RAII-style
// mechanism for owning a mutex for the duration of a scoped block.
template<typename T>
class ScopeSyncLock :public NonCopyable
{
public:
	// Constructor
	ScopeSyncLock(Synchronized<T>& sync_variable) : sync(sync_variable)
	                                              , object(sync_variable.Lock())
	                                              , locked(true)
	{ }

	// Destructor
	~ScopeSyncLock()
	{
		if (locked)
		{
			sync.Unlock();
		}
	}

	// Returns a reference to the locked object
	T& Get()
	{
		return object;
	}

	// Returns a constant reference to the locked object
	const T& Get() const
	{
		return object;
	}

	// Copies locked object from the provided source object
	void Set(const T& copy)
	{
		object = copy;
	}

	// Unlocks the scope. Call when you don't need the lock
	// no more in the current scope
	void Unlock()
	{
		if (locked)
		{
			locked = false;
			sync.Unlock();
		}
	}

private:
	Synchronized<T>& sync;
	T& object;
	bool locked;
};

//----------------------------------------------------------------------------//
// ut::ScopeSyncRWLock is a ut::SyncRW wrapper that provides a convenient
// RAII-style mechanism for owning a mutex for the duration of a scoped block.
template<typename T>
class ScopeSyncRWLock : public NonCopyable
{
public:
	typedef RWLock::Access Access;

	// Constructor
	ScopeSyncRWLock(SyncRW<T>& sync_variable,
	                Access sync_access) : access(sync_access)
	                                    , sync(sync_variable)
	                                    , object(sync_variable.Lock(sync_access))
	                                    , locked(true)
	{ }

	// Destructor
	~ScopeSyncRWLock()
	{
		if (locked)
		{
			sync.Unlock(access);
		}
	}

	// Returns a reference to the locked object
	T& Get()
	{
		return object;
	}

	// Returns a constant reference to the locked object
	const T& Get() const
	{
		return object;
	}

	// Copies locked object from the provided source object
	void Set(const T& copy)
	{
		object = copy;
	}

	// Unlocks the scope. Call when you don't need the lock
	// no more in the current scope
	void Unlock()
	{
		if (locked)
		{
			locked = false;
			sync.Unlock(access);
		}
	}

private:
	Access access;
	SyncRW<T>& sync;
	T& object;
	bool locked;
};

//----------------------------------------------------------------------------//
// ut::SyncPoint is a class to synchronize different events from different
// threads in one specific moment of time defined by ut::SyncPoint::Synchronize
// function call. One can issue synchronization by calling
// ut::SyncPoint::AcquireLock(), and it will wait for the next
// ut::SyncPoint::Synchronize() call, on the other hand SyncPoint::Synchronize()
// function will wait until the lock will be released.
class SyncPoint
{
private:
	// Id of the thread being synchronized with in the moment.
	typedef Optional<ThreadId> OccupantId;

	// Synchronization request.
	class Request
	{
	public:
		// Constructor.
		Request(WeakPtr<OccupantId> in_occupant_thread);

		// Move constructor.
		Request(Request&&) noexcept;

		// Copying is prohibited.
		Request(const Request&) = delete;
		Request& operator = (const Request&) = delete;

		// Move assignment is prohibited.
		Request& operator = (Request&&) = delete;

		// Waits for the synchronization with ut::SyncPoint::Synchronize().
		void Synchronize();

		// Informs issuer that synchronization started and waits for the request
		// to call ut::SyncPoint::Request::Finish().
		void StartAndWaitCompletion(Mutex& occupant_mutex);

		// Informs synchronization point that synchronization is over.
		void Finish();

	private:
		// synchronization primitives
		Mutex mutex;
		ConditionVariable cvar;
		bool active;

		// id of the thread that issued a request
		ThreadId thread_id;

		// id of the current thread occupying synchronization
		WeakPtr<OccupantId> occupant_thread;
	};

public:
	// Lock that is returned by ut::SyncPoint::AcquireLock() is used to control
	// time of the synchronization by issuer. One can call
	// ut::SyncPoint::Lock::Release to finish synchronization.
	class Lock
	{
	public:
		// Constructor.
		Lock(WeakPtr<Request> in_request);

		// Move constructor.
		Lock(Lock&& other) noexcept;

		// Copying is prohibited.
		Lock(const Lock&) = delete;
		Lock& operator = (const Lock&) = delete;

		// Move assignment is prohibited.
		Lock& operator = (Lock&&) = delete;

		// Destructor.
		~Lock();

		// Finishes synchronization process.
		void Release();

	private:
		WeakPtr<Request> request;
	};

	// Constructor.
	SyncPoint();

	// Move constructor.
	SyncPoint(SyncPoint&& other) noexcept;

	// Copying is prohibited.
	SyncPoint(const SyncPoint&) = delete;
	SyncPoint& operator = (const SyncPoint&) = delete;

	// Synchronizes all pending requests. This function is thread-safe.
	void Synchronize();

	// Waits for the synchronization with ut::SyncPoint::Synchronize()
	// and returns a lock controlling synchronization time.
	Lock AcquireLock();

private:
	// synchronization requests
	Synchronized< Array< SharedPtr<Request, thread_safety::Mode::on> > > requests;

	// occupant thread - is a thread being synchronized with in the moment,
	// it must be cached to prevent dead lock in recursion cases
	Mutex occupant_mutex;
	SharedPtr<OccupantId> occupant_thread;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//