//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_throw_error.h"
#include "containers/ut_array.h"
#include "containers/ut_pair.h"
#include "containers/ut_map.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_base_of.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Default deleter for unique smart pointer. Just calls a delete operator.
template <typename T>
class UniquePtrDefaultDeleter
{
public:
	void operator()(T* ptr) const
	{
		delete ptr;
	}
};

//----------------------------------------------------------------------------//
// ut::UniquePtr is a smart pointer that owns and manages another object through
// a pointer and disposes of that object when ut::UniquePtr goes out of scope
template<typename T, typename Deleter = UniquePtrDefaultDeleter<T>>
class UniquePtr : public NonCopyable, private Deleter
{
typedef T ElementType;
public:
	// Default constructor
	constexpr UniquePtr() : pointer(nullptr)
	{}

	// Constructor, takes ownership of provided pointe,
	// you should never use @ptr after passing it here
	//    @param ptr - object's pointer
	explicit UniquePtr(ElementType* ptr) : pointer(ptr)
	{}

	// Move constructor
	//    @param other - another ut::UniquePtr object to copy from
	//                  @other will be discarded after copying
	UniquePtr(UniquePtr&& other) noexcept : pointer(other.Discard())
	{}

	// Move constructor, takes derived type.
	//    @param other - another ut::UniquePtr object to copy from
	//                   @other will be discarded after copying
	template<typename Drv, typename Del, typename = typename EnableIf<IsBaseOf<T, Drv>::value>::Type>
	UniquePtr(UniquePtr<Drv, Del>&& other) noexcept : pointer(other.Discard())
	{}

	// Destructor, deletes @pointer object
	~UniquePtr()
	{
		if (pointer != nullptr)
		{
			GetDeleter()(pointer);
		}
	}

	// Move operator
	//    @param other - another ut::UniquePtr object to move data from
	//                   @other will be discarded after copying
	UniquePtr& operator = (UniquePtr&& other) noexcept
	{
		Switch(other.Discard());
		return *this;
	}

	// Move operator
	//    @param copy - another ut::UniquePtr object to move data from
	//                  @other will be discarded after copying
	template<typename Drv, typename Del>
	typename EnableIf<IsBaseOf<T, Drv>::value, UniquePtr&>::Type operator = (UniquePtr<Drv, Del>&& other) noexcept
	{
		Switch(other.Discard());
		return *this;
	}

	// Overloaded inheritance operator, provides access to the owned object
	ElementType* operator -> ()
	{
		UT_ASSERT(pointer != nullptr);
		return pointer;
	}

	// Overloaded inheritance operator, provides read access to the owned object
	const ElementType* operator -> () const
	{
		UT_ASSERT(pointer != nullptr);
		return pointer;
	}

	// Returns the @pointer object, equivalent to *Get()
	ElementType& operator* ()
	{
		UT_ASSERT(pointer != nullptr);
		return *pointer;
	}

	// Returns a pointer to the managed object or nullptr if no object is owned
	ElementType* Get() const
	{
		return pointer;
	}

	// Returns the reference to the managed object, this function is unsafe,
	// check @pointer value before calling
	ElementType& GetRef()
	{
		UT_ASSERT(pointer != NULL);
		return *pointer;
	}

	// Returns constant reference to the managed object, this function is unsafe,
	// check @pointer value before calling
	const ElementType& GetRef() const
	{
		UT_ASSERT(pointer != nullptr);
		return *pointer;
	}

	// Bool conversion operator
	operator bool() const
	{
		return pointer != nullptr;
	}

	// Returns a reference to the deleter subobject.
	//    @return - a reference to the deleter.
	Deleter& GetDeleter()
	{
		return static_cast<Deleter&>(*this);
	}

	// Resets the managed object, sets @pointer to null
	//    @return - old managed @pointer
	ElementType* Discard()
	{
		ElementType* _tmp = pointer;
		pointer = 0;
		return _tmp;
	}

	// Replaces managed object with new one (@ptr object)
	// old object is deleted
	//    @param ptr - old @pointer
	void Switch(ElementType* ptr = nullptr) noexcept
	{
		if (ptr != pointer)
		{
			if (pointer != nullptr)
			{
				GetDeleter()(pointer);
			}
			pointer = ptr;
		}
	}

	// Deletes @pointer object and sets it to null
	void Delete()
	{
		if (pointer != nullptr)
		{
			GetDeleter()(pointer);
		}
		pointer = nullptr;
	}

	// Returns pointer to the managed memory. Then @pointer is set to zero,
	// so the managed memory is released without deletion.
	//    @return - pointer to the managed object. Caller is responsible for
	//              managing this object after the call.
	ElementType* Release()
	{
		ElementType* tmp = pointer;
		pointer = nullptr;
		return tmp;
	}

protected:
	// Managed object, null by default
	ElementType* pointer;
};

//----------------------------------------------------------------------------//
// Comparison operators for ut::UniquePtr
template <class T1, class T2>
inline bool operator == (const UniquePtr<T1>& left, const UniquePtr<T2>& right)
{ return left.Get() == right.Get(); }

template <class T1, class T2>
inline bool operator != (const UniquePtr<T1>& left, const UniquePtr<T2>& right)
{ return left.Get() != right.Get(); }

template <class T1, class T2>
inline bool operator > (const UniquePtr<T1>& left, const UniquePtr<T2>& right)
{ return left.Get() > right.Get(); }

template <class T1, class T2>
inline bool operator >= (const UniquePtr<T1>& left, const UniquePtr<T2>& right)
{ return left.Get() >= right.Get(); }

template <class T1, class T2>
inline bool operator < (const UniquePtr<T1>& left, const UniquePtr<T2>& right)
{ return left.Get() < right.Get(); }

template <class T1, class T2>
inline bool operator <= (const UniquePtr<T1>& left, const UniquePtr<T2>& right)
{ return left.Get() <= right.Get(); }


//----------------------------------------------------------------------------//
// Comparison operators for ut::UniquePtr with nullptr_t type
template <class T> inline bool operator == (const UniquePtr<T>& left, nullptr_t)
{ return left.Get() == nullptr; }

template <class T> inline bool operator == (nullptr_t, const UniquePtr<T>& right)
{ return nullptr == right.Get(); }

template <class T> inline bool operator != (const UniquePtr<T>& left, nullptr_t)
{ return left.Get() != nullptr; }

template <class T> inline bool operator != (nullptr_t, const UniquePtr<T>& right)
{ return nullptr != right.Get(); }

template <class T> inline bool operator > (const UniquePtr<T>& left, nullptr_t)
{ return left.Get() > nullptr; }

template <class T> inline bool operator > (nullptr_t, const UniquePtr<T>& right)
{ return nullptr > right.Get(); }

template <class T> inline bool operator >= (const UniquePtr<T>& left, nullptr_t)
{ return left.Get() >= nullptr; }

template <class T> inline bool operator >= (nullptr_t, const UniquePtr<T>& right)
{ return nullptr >= right.Get(); }

template <class T> inline bool operator < (const UniquePtr<T>& left, nullptr_t)
{ return left.Get() < nullptr; }

template <class T> inline bool operator < (nullptr_t, const UniquePtr<T>& right)
{ return nullptr < right.Get(); }

template <class T> inline bool operator <= (const UniquePtr<T>& left, nullptr_t)
{ return left.Get() <= nullptr; }

template <class T> inline bool operator <= (nullptr_t, const UniquePtr<T>& right)
{ return nullptr <= right.Get(); }

//----------------------------------------------------------------------------//
// Constructs an object of specified type and wraps it in a ut::UniquePtr
template<typename T, typename... Args>
inline UniquePtr<T> MakeUnique(Args&&...args)
{
	return UniquePtr<T>(new T(Forward<Args>(args)...));
}

//----------------------------------------------------------------------------//
// Specialize type name function for unique ptr
template <typename T, typename Deleter> struct Type< UniquePtr<T, Deleter> >
{
	static inline const char* Name() { return "unique_ptr"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//