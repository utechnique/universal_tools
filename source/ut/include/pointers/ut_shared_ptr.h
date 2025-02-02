//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "pointers/ut_ref_counter.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_base_of.h"
#include "templates/ut_are_types_equal.h"
#include "templates/ut_are_values_equal.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Forward Declaration for the ut::WeakPtr template class.
template<class T, thread_safety::Mode> class WeakPtr;

//----------------------------------------------------------------------------//
// Default thread safety mode for ut::SharedPtr.
static const thread_safety::Mode skDefaultSharedPtrMode = thread_safety::Mode::on;

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
// Also ut::SharedPtr can be used with an incomplete type.
template <typename ObjectType,
          thread_safety::Mode thread_safety_mode = skDefaultSharedPtrMode,
          typename Deleter = DefaultSharedDeleter<ObjectType> >
class SharedPtr
{
	typedef ReferenceController<thread_safety_mode> Controller;

	// WeakPtr is a friend to get access to the reference controller.
	template <typename, thread_safety::Mode> friend class WeakPtr;

	// SharedPtr must have access to other template instantiations.
	template <typename, thread_safety::Mode, typename> friend class SharedPtr;

	// Helper structure to discover if another shared pointer
	// is convertible to the current one.
	template<typename Drv, thread_safety::Mode drv_mode>
	struct IsConvertible
	{
		enum
		{
			value = IsBaseOf<ObjectType, Drv>::value &&
			AreValuesEqual<thread_safety::Mode, thread_safety_mode, drv_mode>::value
		};
	};
public:
	// Default constructor
	SharedPtr() : object(nullptr), referencer()
	{}

	// Constructor. Creates a new reference controller from the raw pointer.
	explicit SharedPtr(ObjectType* managed_object) : object(managed_object)
	                                               , referencer(CreateController(managed_object))
	{}

	// Copy constructor. Copies referencer object. Reference count is increased by 1 here.
	SharedPtr(const SharedPtr& copy) : object(copy.object)
	                                 , referencer(copy.referencer)
	{}

	// Copy constructor, takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode, typename DrvDel,
	         typename = typename EnableIf<IsConvertible<Drv, drv_mode>::value>::Type>
	SharedPtr(const SharedPtr<Drv, drv_mode, DrvDel>& copy)  : object(copy.object)
	                                                         , referencer(copy.referencer)
	{}

	// Move constructor. Moves referencer object. Reference count doesn't change here.
	SharedPtr(SharedPtr&& rval) noexcept : object(rval.object)
	                                     , referencer(Move(rval.referencer))
	{}

	// Move constructor, takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode, typename DrvDel,
	         typename = typename EnableIf<IsConvertible<Drv, drv_mode>::value>::Type>
	SharedPtr(SharedPtr<Drv, drv_mode, DrvDel>&& rval) noexcept : object(rval.object)
	                                                            , referencer(Move(rval.referencer))
	{}

	// Assign operator. Reference count to the old object is decreased by 1 here, and
	// reference count to the new object is increased by 1.
	SharedPtr& operator=(const SharedPtr& copy)
	{
		Reset(copy);
		return *this;
	}

	// Assign operator. Takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode, typename DrvDel>
	typename EnableIf<IsConvertible<Drv, drv_mode>::value, SharedPtr&>::Type
		operator=(const SharedPtr<Drv, drv_mode, DrvDel>& copy)
	{
		Reset(copy);
		return *this;
	}

	// Move operator. Behaves exactly as assign operator (with full-copy behaviour).
	SharedPtr& operator=(SharedPtr&& rval) noexcept
	{
		Reset(rval);
		return *this;
	}

	// Move operator. Takes derived type.
	template<typename Drv, thread_safety::Mode drv_mode, typename DrvDel>
	typename EnableIf<IsConvertible<Drv, drv_mode>::value, SharedPtr&>::Type
		operator=(SharedPtr<Drv, drv_mode, DrvDel>&& rval) noexcept
	{
		Reset(rval);
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

	// Bool conversion operator
	operator bool() const
	{
		return object != nullptr;
	}

	// Recreates referencer object, it decrements the old reference count while destructing,
	// and increments the new reference count while constructing.
	void Reset(ObjectType* obj = nullptr)
	{
		DestructReferencer();
		new(&referencer) SharedReferencer<thread_safety_mode>(CreateController(obj));
		object = obj;
	}

	// Recreates referencer object, it decrements the old reference count while destructing,
	// and increments the new reference count while constructing.
	void Reset(const SharedPtr& copy)
	{
		DestructReferencer();
		new(&referencer) SharedReferencer<thread_safety_mode>(copy.referencer);
		object = copy.object;
	}

private:
	// Creates reference controller with deleter.
	//    @param obj_ptr - pointer to the managed object.
	static inline Controller* CreateController(ObjectType* obj_ptr)
	{
		return new RefControllerWithDeleter<ObjectType, thread_safety_mode, Deleter>(obj_ptr);
	}

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
	SharedReferencer<thread_safety_mode> referencer;
};

//----------------------------------------------------------------------------//
// Comparison operators for ut::SharedPtr
template <class T1, class T2>
inline bool operator == (const SharedPtr<T1>& left, const SharedPtr<T2>& right)
{ return left.Get() == right.Get(); }

template <class T1, class T2>
inline bool operator != (const SharedPtr<T1>& left, const SharedPtr<T2>& right)
{ return left.Get() != right.Get(); }

template <class T1, class T2>
inline bool operator > (const SharedPtr<T1>& left, const SharedPtr<T2>& right)
{ return left.Get() > right.Get(); }

template <class T1, class T2>
inline bool operator >= (const SharedPtr<T1>& left, const SharedPtr<T2>& right)
{ return left.Get() >= right.Get(); }

template <class T1, class T2>
inline bool operator < (const SharedPtr<T1>& left, const SharedPtr<T2>& right)
{ return left.Get() < right.Get(); }

template <class T1, class T2>
inline bool operator <= (const SharedPtr<T1>& left, const SharedPtr<T2>& right)
{ return left.Get() <= right.Get(); }


//----------------------------------------------------------------------------//
// Comparison operators for ut::SharedPtr with nullptr_t type
template <class T> inline bool operator == (const SharedPtr<T>& left, nullptr_t)
{ return left.Get() == nullptr; }

template <class T> inline bool operator == (nullptr_t, const SharedPtr<T>& right)
{ return nullptr == right.Get(); }

template <class T> inline bool operator != (const SharedPtr<T>& left, nullptr_t)
{ return left.Get() != nullptr; }

template <class T> inline bool operator != (nullptr_t, const SharedPtr<T>& right)
{ return nullptr != right.Get(); }

template <class T> inline bool operator > (const SharedPtr<T>& left, nullptr_t)
{ return left.Get() > nullptr; }

template <class T> inline bool operator > (nullptr_t, const SharedPtr<T>& right)
{ return nullptr > right.Get(); }

template <class T> inline bool operator >= (const SharedPtr<T>& left, nullptr_t)
{ return left.Get() >= nullptr; }

template <class T> inline bool operator >= (nullptr_t, const SharedPtr<T>& right)
{ return nullptr >= right.Get(); }

template <class T> inline bool operator < (const SharedPtr<T>& left, nullptr_t)
{ return left.Get() < nullptr; }

template <class T> inline bool operator < (nullptr_t, const SharedPtr<T>& right)
{ return nullptr < right.Get(); }

template <class T> inline bool operator <= (const SharedPtr<T>& left, nullptr_t)
{ return left.Get() <= nullptr; }

template <class T> inline bool operator <= (nullptr_t, const SharedPtr<T>& right)
{ return nullptr <= right.Get(); }

//----------------------------------------------------------------------------//
// Constructs an object of specified type and wraps it in a ut::SharedPtr
template<typename T, typename... Args>
inline SharedPtr<T> MakeShared(Args&&...args)
{
	return SharedPtr<T, skDefaultSharedPtrMode>(new T(Forward<Args>(args)...));
}

//----------------------------------------------------------------------------//
// The same as ut::MakeShared(), but resulting pointer is guaranteed to have
// thread safety enabled.
template<typename T, typename... Args>
inline SharedPtr<T, thread_safety::Mode::on> MakeSafeShared(Args&&...args)
{
	return SharedPtr<T, thread_safety::Mode::on>(new T(Forward<Args>(args)...));
}

//----------------------------------------------------------------------------//
// The same as ut::MakeShared(), but resulting pointer is guaranteed to have
// thread safety disabled.
template<typename T, typename... Args>
inline SharedPtr<T, thread_safety::Mode::off> MakeUnsafeShared(Args&&...args)
{
	return SharedPtr<T, thread_safety::Mode::off>(new T(Forward<Args>(args)...));
}

//----------------------------------------------------------------------------//
// Specialize type name function for shared ptr
template <typename T, thread_safety::Mode mode, typename Deleter>
struct Type< SharedPtr<T, mode, Deleter> >
{
	static inline const char* Name() { return "shared_ptr"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//