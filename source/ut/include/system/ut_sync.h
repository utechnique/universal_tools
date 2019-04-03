//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "system/ut_mutex.h"
#include "system/ut_lock.h"
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
class Synchronized
{
public:
	// Default constructor
	Synchronized() {}

	// Copy constructor, @object is safely extracted from @copy
	Synchronized(Synchronized& copy) : object(copy.Get()) {}

	// Copy constructor, @object is explicitly copied from @copy
	Synchronized(typename LValRef<T>::Type copy) : object(copy) {}

	// Move constructor, @object is explicitly copied from @copy
#if CPP_STANDARD >= 2011
	Synchronized(T&& rval) : object(Move(rval)) {}
#endif

	// Assignment operator, @object is safely extracted from @copy
	Synchronized& operator = (Synchronized& copy)
	{
		Set(copy.Get());
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
	void Set(typename LValRef<T>::Type copy)
	{
		ScopeLock scl(m);
		object = copy;
	}

	// Setter function, @object is moved from @rval
#if CPP_STANDARD >= 2011
	void Set(T&& rval)
	{
		ScopeLock scl(m);
		object = Move(rval);
	}
#endif

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
	// Default constructor
	SyncRW() {}

	// Copy constructor, @object is safely extracted from @copy
	SyncRW(SyncRW& copy) : object(copy.Get()) {}

	// Copy constructor, @object is explicitly copied from @copy
	SyncRW(const T& copy) : object(copy) {}

	// Move constructor, @object is explicitly copied from @copy
#if CPP_STANDARD >= 2011
	SyncRW(T&& rval) : object(Move(rval)) {}
#endif

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
		ScopeRWLock scl(lock, access_read);
		return object;
	}

	// Locks mutex @m, and returns @object reference
	T& Lock(Access access = access_write)
	{
		lock.Lock(access);
		return object;
	}

	// Unlocks mutex @m
	void Unlock(Access access = access_write)
	{
		lock.Unlock(access);
	}

	// Setter function, @object is safely extracted from @copy
	void Set(const T& copy)
	{
		ScopeRWLock scl(lock, access_write);
		object = copy;
	}

	// Setter function, @object is moved from @rval
#if CPP_STANDARD >= 2011
	void Set(T&& rval)
	{
		ScopeRWLock scl(lock, access_write);
		object = Move(rval);
	}
#endif

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
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//