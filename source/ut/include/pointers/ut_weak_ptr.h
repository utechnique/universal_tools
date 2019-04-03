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

template<typename ObjectType>
class WeakPtr
{
	// SharedPtr is a friend to get access to the reference controller.
	template <typename> friend class SharedPtr;
public:
	// Default constructor.
	WeakPtr() : object(nullptr), referencer(nullptr)
	{ }

	// Copy constructor. Copies referencer object. Reference count is increased by 1 here.
	WeakPtr(const WeakPtr& copy) : object(copy.object)
	                             , referencer(copy.referencer)
	{ }

	// Move constructor. Moves referencer object. Reference count doesn't change here.
#if CPP_STANDARD >= 2011
	WeakPtr(WeakPtr&& rval) : object(rval.object)
	                        , referencer(Move(rval.referencer))
	{ }
#endif

	// Constructor from shared pointer.
	WeakPtr(const SharedPtr<ObjectType>& copy) : object(copy.object)
	                                           , referencer(copy.referencer)
	{ }

	// Assign operator. Reference count to the old object is decreased by 1 here, and
	// reference count to the new object is increased by 1.
	WeakPtr& operator=(const WeakPtr& copy)
	{
		Reset< WeakPtr<ObjectType> >(copy);
		return *this;
	}

	// Move operator. Behaves exactly as assign operator (with full-copy behaviour).
#if CPP_STANDARD >= 2011
	WeakPtr& operator=(WeakPtr&& rval)
	{
		Reset< WeakPtr<ObjectType> >(rval);
		return *this;
	}
#endif

	// Assign operator. Takes shared pointer here.
	WeakPtr& operator=(const SharedPtr<ObjectType>& copy)
	{
		Reset< SharedPtr<ObjectType> >(copy);
		return *this;
	}

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

	// Tests to see whether or not this weak counter contains a valid reference
	// @return - true if reference is valid
	const bool IsValid() const
	{
		return object != nullptr && referencer.IsValid();
	}

	// Converts this weak pointer to a shared pointer that you can use to access the object (if it
	// hasn't expired yet.) Remember, if there are no more shared references to the object, the
	// returned shared pointer will not be valid. You should always check to make sure the returned
	// pointer is valid before trying to dereference the shared pointer!
	//    @return - Shared pointer for this object (will only be valid if still referenced!)
	// 
	SharedPtr<ObjectType> Pin() const
	{
		return SharedPtr<ObjectType>(*this);
	}

	// Recreates referencer object, it decrements the old reference count while destructing,
	// and increments the new reference count while constructing.
	template <typename PointerType>
	void Reset(const PointerType& copy)
	{
		DestructReferencer();
		new(&referencer) WeakReferencer<ObjectType>(copy.referencer);
		object = copy.object;
	}

private:
	// Calls destructor of the referencer object. Reference count is decremented here.
	void DestructReferencer()
	{
		referencer.~WeakReferencer();
	}

	// Managed object, null by default
	ObjectType* object;

	// Referencer for the weak pointers, contains a pointer to the reference controller.
	// WeakReferencer<> class is a convenient wrapper, that uses reference controller
	// to increment reference count on construction and decrement this count on destruction.
	WeakReferencer<ObjectType> referencer;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//