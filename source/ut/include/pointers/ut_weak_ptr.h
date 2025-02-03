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
// template class ut::WeakPtr is a smart pointer that holds a non - owning("weak")
// reference to an object that is managed by ut::SharedPtr. It must be converted
// to ut::SharedPtr in order to access the referenced object.
template<typename ObjectType, thread_safety::Mode thread_safety_mode = thread_safety::Mode::on>
class WeakPtr
{
	// SharedPtr is a friend to get access to the reference controller.
	template <typename, thread_safety::Mode, typename> friend class SharedPtr;

	// WeakPtr must have access to other template instantiations.
	template <typename, thread_safety::Mode> friend class WeakPtr;

	// Type of corresponding shared pointer.
	typedef SharedPtr<ObjectType, thread_safety_mode> SharedPtrType;
public:
	// Default constructor.
	WeakPtr() : object(nullptr), referencer(nullptr)
	{}

	// Copy constructor. Copies referencer object. Reference count is increased by 1 here.
	WeakPtr(const WeakPtr& copy) : object(copy.object)
	                             , referencer(copy.referencer)
	{}

	// Copy constructor, takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode,
	         typename = typename EnableIf<SharedPtrType::template IsConvertible<Drv, drv_mode>::value>::Type>
	WeakPtr(const WeakPtr<Drv, drv_mode>& copy) : object(copy.object)
	                                            , referencer(copy.referencer)
	{}

	// Move constructor. Moves referencer object. Reference count doesn't change here.
	WeakPtr(WeakPtr&& rval) noexcept : object(rval.object)
	                                 , referencer(Move(rval.referencer))
	{}

	// Move constructor, takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode,
	         typename = typename EnableIf<SharedPtrType::template IsConvertible<Drv, drv_mode>::value>::Type>
	WeakPtr(WeakPtr<Drv, drv_mode>&& rval) noexcept : object(rval.object)
	                                                , referencer(Move(rval.referencer))
	{}

	// Constructor from shared pointer.
	WeakPtr(const SharedPtr<ObjectType, thread_safety_mode>& copy) : object(copy.object)
	                                                               , referencer(copy.referencer)
	{}

	// Assign operator. Reference count to the old object is decreased by 1 here, and
	// reference count to the new object is increased by 1.
	WeakPtr& operator=(const WeakPtr& copy)
	{
		Reset< WeakPtr<ObjectType, thread_safety_mode> >(copy);
		return *this;
	}

	// Assign operator. Takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode>
	typename EnableIf<SharedPtrType::template IsConvertible<Drv, drv_mode>::value, WeakPtr&>::Type
		operator=(const WeakPtr<Drv, drv_mode>& copy)
	{
		Reset< WeakPtr<ObjectType, thread_safety_mode> >(copy);
		return *this;
	}

	// Move operator. Behaves exactly as assign operator (with full-copy behaviour).
	WeakPtr& operator=(WeakPtr&& rval) noexcept
	{
		Reset< WeakPtr<ObjectType, thread_safety_mode> >(rval);
		return *this;
	}

	// Move operator. Takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode>
	typename EnableIf<SharedPtrType::template IsConvertible<Drv, drv_mode>::value, WeakPtr&>::Type
		operator=(WeakPtr<Drv, drv_mode>&& rval) noexcept
	{
		Reset< WeakPtr<ObjectType, thread_safety_mode> >(rval);
		return *this;
	}

	// Assign operator. Takes shared pointer here.
	WeakPtr& operator=(const SharedPtr<ObjectType, thread_safety_mode>& copy)
	{
		Reset< SharedPtr<ObjectType, thread_safety_mode> >(copy);
		return *this;
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
	SharedPtr<ObjectType, thread_safety_mode> Pin() const
	{
		return SharedPtr<ObjectType, thread_safety_mode>(*this);
	}

	// Recreates referencer object, it decrements the old reference count while destructing,
	// and increments the new reference count while constructing.
	template <typename PointerType>
	void Reset(const PointerType& copy)
	{
		DestructReferencer();
		new(&referencer) WeakReferencer<thread_safety_mode>(copy.referencer);
		object = copy.object;
	}

	// Decrements reference count, destructs referencer object and releases managed pointer
	void Reset()
	{
		DestructReferencer();
		new(&referencer) WeakReferencer<thread_safety_mode>(nullptr);
		object = nullptr;
	}

	// Bool conversion operator
	operator bool() const
	{
		return object != nullptr;
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
	WeakReferencer<thread_safety_mode> referencer;
};

// Specialize type name function for weak ptr
template <typename T, thread_safety::Mode mode>
struct Type< WeakPtr<T, mode> >
{
	static inline const char* Name() { return "weak_ptr"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//