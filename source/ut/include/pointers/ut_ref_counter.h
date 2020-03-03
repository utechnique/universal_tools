//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "system/ut_interlocked.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Default deleter for a shared object. Just calls a delete operator.
template <typename T>
class DefaultSharedDeleter
{
public:
	void operator()(T* ptr) const
	{
		delete ptr;
	}
};

//----------------------------------------------------------------------------//
// ut::SharedHolder is a base class for both thread-safe and thread-unsafe
// variants of a reference controller. It takes ownership of the managed object
// and is responsible for the deletion of this object.
template <typename ObjectType>
class SharedHolder
{
public:
	// Constructor. Takes control of the managed object.
	SharedHolder(ObjectType *in_object) : object(in_object)
	{ }

	// Destroys managed object.
	virtual void DestroyObject() = 0;

protected:
	ObjectType *object;
};

//----------------------------------------------------------------------------//
// ut::RefControllerBase is a base class for both thread-safe and thread-unsafe
// variants of a reference controller. It owns count of both weak and strong
// references.
class RefControllerBase : public NonCopyable
{
public:
	// Constructor. Note that both strong (shared) and weak reference count
	// is set to 1.
	RefControllerBase() : strong_count(1), weak_count(1)
	{ }

protected:
	int32 strong_count;
	int32 weak_count;
};

//----------------------------------------------------------------------------//
// ut::ReferenceController is a class incapsulating reference counting features.
// It is responsible for self-destruction if weak reference count becomes zero
// and destroying managed object if strong (shared) count becomes zero.
template <typename ObjectType, thread_safety::Mode mode>
class ReferenceController;

// Thread-safe variant of a reference controller.
template <typename ObjectType>
class ReferenceController<ObjectType, thread_safety::on> : public SharedHolder<ObjectType>
                                                         , public RefControllerBase
{
public:
	// SharedReferencer and WeakReferencer are made friends, so that
	// self-deletion could be performed only from the reliable source.
	template <typename, thread_safety::Mode> friend class SharedReferencer;
	template <typename, thread_safety::Mode> friend class WeakReferencer;

	// Constructor. Note that both strong (shared) and weak reference count
	// is set to 1. Also controller takes control of the managed object.
	ReferenceController(ObjectType *in_object) : SharedHolder<ObjectType>(in_object)
	{ }

private:
	// Increments strong (shared) reference count.
	void AddStrongRef()
	{
		atomics::interlocked::Increment(&this->strong_count);
	}

	// Adds a shared reference to this counter ONLY if there is already at least one reference
	// @return  True if the shared reference was added successfully
	bool ConditionallyAddStrongRef()
	{
		for (; ; )
		{
			// Peek at the current shared reference count.  Remember, this value may be updated by
			// multiple threads.
			const int32 original_count = atomics::interlocked::Read((int32 volatile*)&this->strong_count);
			if (original_count == 0)
			{
				// Never add a shared reference if the pointer has already expired
				return false;
			}

			// Attempt to increment the reference count.
			const int32 cmp_result = atomics::interlocked::CompareExchange(&this->strong_count,
			                                                               original_count + 1,
			                                                               original_count);

			// We need to make sure that we never revive a counter that has already expired, so
			// if the actual value what we expected (because it was touched by another thread),
			// then we'll try again. Note that only in very unusual cases will this actually have
			// to loop.
			if (cmp_result == original_count)
			{
				return true;
			}
		}
	}

	// Increments weak reference count.
	void AddWeakRef()
	{
		atomics::interlocked::Increment(&this->weak_count);
	}

	// Decrements strong (shared) reference count.
	void ReleaseStrongRef()
	{
		if (atomics::interlocked::Decrement(&this->strong_count) == 0)
		{
			// last shared reference was released!
			DestroyObject();

			// No more shared referencers, so decrement the weak reference count by one.
			// When the weak reference count reaches zero, this object will be deleted.
			ReleaseWeakRef();
		}
	}

	// Decrements weak reference count.
	void ReleaseWeakRef()
	{
		if (atomics::interlocked::Decrement(&this->weak_count) == 0)
		{
			// No more references to this reference count.  Destroy it!
			delete this;
		}
	}

	// Returns the shared reference count
	const int32 GetStrongRefCount()
	{
		// This reference count may be accessed by multiple threads
		return atomics::interlocked::Read((int32 volatile*)&this->strong_count);
	}

	// Destroys managed object.
	virtual void DestroyObject() = 0;
};

// Unsafe variant of a reference controller.
template <typename ObjectType>
class ReferenceController<ObjectType, thread_safety::off> : public SharedHolder<ObjectType>
                                                          , public RefControllerBase
{
public:
	// SharedReferencer and WeakReferencer are made friends, so that
	// self-deletion could be performed only from the reliable source.
	template <typename, thread_safety::Mode> friend class SharedReferencer;
	template <typename, thread_safety::Mode> friend class WeakReferencer;

	// Constructor. Note that both strong (shared) and weak reference count
	// is set to 1. Also controller takes control of the managed object.
	ReferenceController(ObjectType *in_object) : SharedHolder<ObjectType>(in_object)
	{ }

private:
	// Increments strong (shared) reference count.
	void AddStrongRef()
	{
		++this->strong_count;
	}

	// Adds a shared reference to this counter ONLY if there is already at least one reference
	// @return  True if the shared reference was added successfully
	bool ConditionallyAddStrongRef()
	{
		if (this->strong_count == 0)
		{
			// Never add a shared reference if the pointer has already expired
			return false;
		}

		++this->strong_count;
		return true;
	}

	// Increments weak reference count.
	void AddWeakRef()
	{
		++this->weak_count;
	}

	// Decrements strong (shared) reference count.
	void ReleaseStrongRef()
	{
		if (--this->strong_count == 0)
		{
			// Last shared reference was released!  Destroy the referenced object.
			DestroyObject();

			// No more shared referencers, so decrement the weak reference count by one.
			// When the weak reference count reaches zero, this object will be deleted.
			ReleaseWeakRef();
		}
	}

	// Decrements weak reference count.
	void ReleaseWeakRef()
	{
		if (--this->weak_count == 0)
		{
			// No more references to this reference count.  Destroy it!
			delete this;
		}
	}

	// Returns the shared reference count
	const int32 GetStrongRefCount()
	{
		// This reference count may be accessed by multiple threads
		return this->strong_count;
	}

	// Destroys managed object.
	virtual void DestroyObject() = 0;
};

//----------------------------------------------------------------------------//
// ut::RefControllerWithDeleter template class is derived from template class
// ut::ReferenceController and overrides it's virtual DestroyObject() method
// using a specified deleter.
template <typename ObjType, thread_safety::Mode mode, typename Deleter = DefaultSharedDeleter<ObjType> >
class RefControllerWithDeleter : public ReferenceController<ObjType, mode>
                               , private Deleter
{
	typedef ReferenceController<ObjType, mode> Base;
public:
	RefControllerWithDeleter(ObjType* object) : Base(object)
	{ }

	void DestroyObject()
	{
		static_cast<Deleter&>(*this)(SharedHolder<ObjType>::object);
	}
};

//----------------------------------------------------------------------------//
// Forward Declaration for the ut::WeakReferencer template class.
template <typename, thread_safety::Mode> class WeakReferencer;

//----------------------------------------------------------------------------//
// ut::SharedReferencer is a wrapper class for managing reference controller
// object for shared pointers. Reference controller is used to increment
// strong (shared) reference count on construction and decrement the count on
// destruction.
template <typename ObjectType, thread_safety::Mode thread_safety_mode>
class SharedReferencer
{
	typedef ReferenceController<ObjectType, thread_safety_mode> Controller;

	// WeakReferencer is a friend to get access to the controller object.
	template <typename, thread_safety::Mode> friend class WeakReferencer;
public:
	// Default constructor
	SharedReferencer() : controller(nullptr)
	{ }

	// Constructor, takes control over reference controller object.
	// @controller supposed to be created using new() operator or to be nullptr.
	SharedReferencer(Controller* in_controller) : controller(in_controller)
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
	SharedReferencer(const WeakReferencer<ObjectType, thread_safety_mode>& weak) : controller(weak.controller)
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
	Controller* controller;
};

//----------------------------------------------------------------------------//
// ut::WeakReferencer is a wrapper class for managing reference controller
// object for weak pointers. Reference controller is used to increment
// weak reference count on construction and decrement the count on destruction.
template <typename ObjectType, thread_safety::Mode thread_safety_mode>
class WeakReferencer
{
	typedef ReferenceController<ObjectType, thread_safety_mode> Controller;
	// SharedReferencer is a friend to get access to the controller object.
	template <typename, thread_safety::Mode> friend class SharedReferencer;
public:
	// Default constructor
	WeakReferencer() : controller(nullptr)
	{ }

	// Constructor, takes control over reference controller object.
	// @controller supposed to be created using new() operator or to be nullptr.
	WeakReferencer(Controller* in_controller) : controller(in_controller)
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
	WeakReferencer(const SharedReferencer<ObjectType, thread_safety_mode>& copy) : controller(copy.controller)
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
	Controller* controller;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//