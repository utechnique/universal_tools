//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "common/ut_atomics.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::ReferenceController is a class incapsulating reference counting features.
// It is responsible for self-destruction if weak reference count becomes zero
// and destroying managed object if strong (shared) count becomes zero.
template <typename ObjectType>
class ReferenceController : public NonCopyable
{
public:
	// SharedReferencer and WeakReferencer are made friends, so that
	// self-deletion could be performed only from the reliable source.
	template <typename> friend class SharedReferencer;
	template <typename> friend class WeakReferencer;

	// Constructor. Note that both strong (shared) and weak reference count
	// is set to 1. Also controller takes control of the managed object.
	ReferenceController(ObjectType *in_object) : strong_count(1)
	                                           , weak_count(1)
	                                           , object(in_object)
	{ }

private:
	// Increments strong (shared) reference count.
	void AddStrongRef()
	{
		atomics::interlocked::Increment(&strong_count);
	}

	// Adds a shared reference to this counter ONLY if there is already at least one reference
	// @return  True if the shared reference was added successfully
	bool ConditionallyAddStrongRef()
	{
		for (; ; )
		{
			// Peek at the current shared reference count.  Remember, this value may be updated by
			// multiple threads.
			const int32 original_count = atomics::interlocked::Read((int32 volatile*)&strong_count);
			if (original_count == 0)
			{
				// Never add a shared reference if the pointer has already expired
				return false;
			}

			// Attempt to increment the reference count.
			const int32 cmp_result = atomics::interlocked::CompareExchange(&strong_count, original_count + 1, original_count);

			// We need to make sure that we never revive a counter that has already expired, so if the
			// actual value what we expected (because it was touched by another thread), then we'll try
			// again. Note that only in very unusual cases will this actually have to loop.
			if (cmp_result == original_count)
			{
				return true;
			}
		}
	}

	// Increments weak reference count.
	void AddWeakRef()
	{
		atomics::interlocked::Increment(&weak_count);
	}

	// Decrements strong (shared) reference count.
	void ReleaseStrongRef()
	{
		if (atomics::interlocked::Decrement(&strong_count) == 0)
		{
			// last shared reference was released!
			DestroyObject();

			// No more shared referencers, so decrement the weak reference count by one.  When the weak
			// reference count reaches zero, this object will be deleted.
			ReleaseWeakRef();
		}
	}

	// Decrements weak reference count.
	void ReleaseWeakRef()
	{
		if (atomics::interlocked::Decrement(&weak_count) == 0)
		{
			// No more references to this reference count.  Destroy it!
			delete this;
		}
	}

	// Returns the shared reference count
	const int32 GetStrongRefCount()
	{
		// This reference count may be accessed by multiple threads
		return atomics::interlocked::Read((int32 volatile*)&strong_count);
	}

	// Destroys managed object.
	void DestroyObject()
	{
		delete object;
	}

private:
	int32 strong_count;
	int32 weak_count;
	ObjectType *object;
};

//----------------------------------------------------------------------------//
// Forward Declaration for the ut::WeakReferencer template class.
template <typename> class WeakReferencer;

//----------------------------------------------------------------------------//
// ut::SharedReferencer is a wrapper class for managing reference controller
// object for shared pointers. Reference controller is used to increment
// strong (shared) reference count on construction and decrement the count on
// destruction.
template <typename ObjectType>
class SharedReferencer
{
	// WeakReferencer is a friend to get access to the controller object.
	template <typename> friend class WeakReferencer;
public:
	// Constructor, takes control over reference controller object.
	// @controller supposed to be created using new() operator or to be nullptr.
	SharedReferencer(ReferenceController<ObjectType>* in_controller) : controller(in_controller)
	{ }

	// Copy constructor, copies a pointer to the reference controller and
	// increments strong (shared) reference count.
	SharedReferencer(const SharedReferencer& copy) : controller(copy.controller)
	{
		if (controller != nullptr)
		{
			controller->AddStrongRef();
		}
	}

	// Move constructor, just copies a pointer to the reference controller
	// without incrementing reference count.
#if CPP_STANDARD >= 2011
	SharedReferencer(SharedReferencer&& rval) : controller(rval.controller)
	{
		rval.controller = nullptr;
	}
#endif

	// Constructor, creates a shared referencer object from a weak referencer object. This will only result
	// in a valid object reference if the object already has at least one other shared referencer.
	SharedReferencer(const WeakReferencer<ObjectType>& weak) : controller(weak.controller)
	{
		// If the incoming reference had an object associated with it, then go ahead and increment the
		// shared reference count
		if (controller != nullptr)
		{
			// Attempt to elevate a weak reference to a shared one. For this to work, the object this
			// weak counter is associated with must already have at least one shared reference.  We'll
			// never revive a pointer that has already expired!
			if (!controller->ConditionallyAddStrongRef())
			{
				controller = nullptr;
			}
		}
	}

	// Destructor, decrements strong (shared) reference count.
	~SharedReferencer()
	{
		if (controller != nullptr)
		{
			controller->ReleaseStrongRef();
		}
	}

	// Tests to see whether or not this weak counter contains a valid reference
	// @return - true if reference is valid
	const bool IsValid() const
	{
		return controller != nullptr;
	}

private:
	// Assign operators are prohibited.
	SharedReferencer& operator=(const SharedReferencer& copy) PROHIBITED;
#if CPP_STANDARD >= 2011
	SharedReferencer& operator=(SharedReferencer&& rval) PROHIBITED;
#endif

	// Reference-counting controller
	ReferenceController<ObjectType>* controller;
};

//----------------------------------------------------------------------------//
// ut::WeakReferencer is a wrapper class for managing reference controller
// object for weak pointers. Reference controller is used to increment
// weak reference count on construction and decrement the count on destruction.
template <typename ObjectType>
class WeakReferencer
{
	// SharedReferencer is a friend to get access to the controller object.
	template <typename> friend class SharedReferencer;
public:
	// Constructor, takes control over reference controller object.
	// @controller supposed to be created using new() operator or to be nullptr.
	WeakReferencer(ReferenceController<ObjectType>* in_controller) : controller(in_controller)
	{ }

	// Copy constructor, copies a pointer to the reference controller and
	// increments weak reference count.
	WeakReferencer(const WeakReferencer& copy) : controller(copy.controller)
	{
		if (controller != nullptr)
		{
			controller->AddWeakRef();
		}
	}

	// Move constructor, just copies a pointer to the reference controller
	// without incrementing reference count.
#if CPP_STANDARD >= 2011
	WeakReferencer(WeakReferencer&& rval) : controller(rval.controller)
	{
		rval.controller = nullptr;
	}
#endif

	// Constructor from the shared pointer, copies a pointer to the
	// reference controller and increments weak reference count.
	WeakReferencer(const SharedReferencer<ObjectType>& copy) : controller(copy.controller)
	{
		if (controller != nullptr)
		{
			controller->AddWeakRef();
		}
	}

	// Destructor, decrements weak reference count.
	~WeakReferencer()
	{
		if (controller != nullptr)
		{
			controller->ReleaseWeakRef();
		}
	}

	// Tests to see whether or not this weak counter contains a valid reference
	// @return - true if reference is valid
	const bool IsValid() const
	{
		return controller != nullptr && controller->GetStrongRefCount() > 0;
	}

private:
	// Assign operators are prohibited.
	WeakReferencer& operator=(const WeakReferencer& copy) PROHIBITED;
#if CPP_STANDARD >= 2011
	WeakReferencer& operator=(WeakReferencer&& rval) PROHIBITED;
#endif

	// Reference-counting controller
	ReferenceController<ObjectType>* controller;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//