//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "pointers/ut_ref_counter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Forward Declaration for the ut::WeakPtr template class.
template<class T, thread_safety::Mode> class WeakPtr;

//----------------------------------------------------------------------------//
// template class ut::SharedPtr is a smart pointer that retains shared ownership
// of an object through a pointer. Several ut::SharedPtr objects may own the
// same object. The object is destroyed and its memory deallocated when either
// of the following happens:
//     - the last remaining ut::SharedPtr owning the object is destroyed;
//     - the last remaining shared_ptr owning the object is assigned another
//       pointer via operator= or Reset().
// A ut::SharedPtr can share ownership of an object while storing a pointer to
// another object.This feature can be used to point to member objects while
// owning the object they belong to.The stored pointer is the one accessed by
// ut::SharedPtr::Get(), the dereference and the comparison operators.The
// managed pointer is the one passed to the deleter when use count reaches zero.
template<typename ObjectType, thread_safety::Mode thread_safety_mode = thread_safety::on>
class SharedPtr
{
	typedef ReferenceController<ObjectType, thread_safety_mode> Controller;
	// WeakPtr is a friend to get access to the reference controller.
	template <typename, thread_safety::Mode> friend class WeakPtr;
public:
	// Constructor. Creates a new reference controller from the raw pointer.
	SharedPtr(ObjectType* managed_object = nullptr) : object(managed_object)
	                                                , referencer(new Controller(managed_object))
	{ }

	// Copy constructor. Copies referencer object. Reference count is increased by 1 here.
	SharedPtr(const SharedPtr& copy) : object(copy.object)
	                                 , referencer(copy.referencer)
	{ }

	// Move constructor. Moves referencer object. Reference count doesn't change here.
#if CPP_STANDARD >= 2011
	SharedPtr(SharedPtr&& rval) : object(rval.object)
	                            , referencer(Move(rval.referencer))
	{ }
#endif

	// Assign operator. Reference count to the old object is decreased by 1 here, and
	// reference count to the new object is increased by 1.
	SharedPtr& operator=(const SharedPtr& copy)
	{
		Reset(copy);
		return *this;
	}

	// Move operator. Behaves exactly as assign operator (with full-copy behaviour).
#if CPP_STANDARD >= 2011
	SharedPtr& operator=(SharedPtr&& rval)
	{
		Reset(rval);
		return *this;
	}
#endif

	// Overloaded inheritance operator, provides access to the owned object
	ObjectType* operator -> ()
	{
		UT_ASSERT(object != nullptr);
		return object;
	}

	// Overloaded inheritance operator, provides read access to the owned object
	const ObjectType* operator -> () const
	{
		UT_ASSERT(object != nullptr);
		return object;
	}

	// Returns the @object object, equivalent to *Get()
	ObjectType& operator* ()
	{
		UT_ASSERT(object != nullptr);
		return *object;
	}

	// Returns a pointer to the managed object or nullptr if no object is owned
	ObjectType* Get() const
	{
		return object;
	}

	// Returns the reference to the managed object, this function is unsafe,
	// check @object value before calling
	ObjectType& GetRef()
	{
		UT_ASSERT(object != nullptr);
		return *object;
	}

	// Returns constant reference to the managed object, this function is unsafe,
	// check @object value before calling
	const ObjectType& GetRef() const
	{
		UT_ASSERT(object != nullptr);
		return *object;
	}

	// Comparison operator, returns 'true' if @pointer is not a null
	bool operator == (bool b_value) const
	{
		return object && b_value;
	}

	// Comparison operator, returns 'true' if @object if has the same address as @ptr
	bool operator == (ObjectType* ptr) const
	{
		return object == ptr;
	}

	// Comparison operator, returns 'false' if @object if has the same address as @ptr
	bool operator != (ObjectType* ptr) const
	{
		return object != ptr;
	}

	// Returns 'true' if both @object and @ptr are non-null
	bool operator && (ObjectType* ptr) const
	{
		return object && ptr;
	}

	// Returns 'true' if both @object is not a null and @b_value is 'true'
	bool operator && (bool b_value) const
	{
		return object && b_value;
	}

	// Returns 'true' if @object is null
	bool operator ! () const
	{
		return !object;
	}

	// Recreates referencer object, it decrements the old reference count while destructing,
	// and increments the new reference count while constructing.
	void Reset(ObjectType* obj = nullptr)
	{
		DestructReferencer();
		new(&referencer) SharedReferencer<ObjectType, thread_safety_mode>(new Controller(obj));
		object = obj;
	}

	// Recreates referencer object, it decrements the old reference count while destructing,
	// and increments the new reference count while constructing.
	void Reset(const SharedPtr& copy)
	{
		DestructReferencer();
		new(&referencer) SharedReferencer<ObjectType, thread_safety_mode>(copy.referencer);
		object = copy.object;
	}

private:
	// Constructs a shared pointer from a weak pointer, allowing you to access the object (if it
	// hasn't expired yet.)  Remember, if there are no more shared references to the object, the
	// shared pointer will not be valid. You should always check to make sure this shared
	// pointer is valid before trying to dereference the shared pointer!
	// NOTE: This constructor is private to force users to be explicit when converting a weak
	//       pointer to a shared pointer. Use the weak pointer's Pin() method instead!
	template <typename OtherType>
	SharedPtr(const WeakPtr<OtherType, thread_safety_mode>& weak) : object(nullptr)
	                                                              , referencer(weak.referencer)
	{
		// Check that the shared reference was created from the weak reference successfully.  We'll only
		// cache a pointer to the object if we have a valid shared reference.
		if (referencer.IsValid())
		{
			object = weak.object;
		}
	}

	// Calls destructor of the referencer object. Reference count is decremented here.
	void DestructReferencer()
	{
		referencer.~SharedReferencer();
	}

	// Managed object, null by default
	ObjectType* object;

	// Referencer for the shared pointers, contains a pointer to the reference controller.
	// SharedReferencer<> class is a convenient wrapper, that uses reference controller
	// to increment reference count on construction and decrement this count on destruction.
	SharedReferencer<ObjectType, thread_safety_mode> referencer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//