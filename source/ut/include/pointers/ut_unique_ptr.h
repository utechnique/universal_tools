//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_throw_error.h"
#include "containers/ut_array.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// struct ut::PtrRefCast is a proxy structure for ut::UniquePtr convertions.
// C++03 doesn't support r-value references and move semantics, so we need
// to implement UniquePtr copy constructor with non-reference (but a full object)
// parameter. This is done via intermediate structure ut::PtrRefCast.
template<typename T>
struct PtrRefCast
{
	explicit PtrRefCast(T* p) : ptr(p) { }
	T* ptr;
};

//----------------------------------------------------------------------------//
// ut::UniquePtr is a smart pointer that owns and manages another object through
// a pointer and disposes of that object when ut::UniquePtr goes out of scope
template<typename T>
class UniquePtr : public NonCopyable
{
typedef T ElementType;
public:
	// Default constructor, you should never use @ptr after passing it here
	//    @param ptr - object's pointer
	UniquePtr(ElementType* ptr = nullptr) : pointer(ptr)
	{ }

	// Copy constructor, for cpp standard older than 2011
	// use ut::Move for newer dialects
	//    @param copy - another ut::Ptr object to copy from
	//                  @copy will be discarded after copying
#if CPP_STANDARD < 2011
	UniquePtr(UniquePtr& copy) : pointer(copy.Discard())
	{ }
#endif

	// Move constructor
	//    @param copy - another ut::Ptr object to copy from
	//                  @copy will be discarded after copying
#if CPP_STANDARD >= 2011
	UniquePtr(UniquePtr && copy) : pointer(copy.Discard())
	{ }
#endif

	// Destructor, deletes @pointer object
	~UniquePtr()
	{
		if (pointer != nullptr)
		{
			delete pointer;
		}
	}

	// Assignment operator
	//    @param copy - another ut::Ptr object to copy from
	//                  @copy will be discarded after copying
#if CPP_STANDARD < 2011
	UniquePtr& operator = (UniquePtr& copy)
	{
		Switch(copy.Discard());
		return *this;
	}
#endif

	// Move operator
	//    @param copy - another ut::Ptr object to copy from
	//                  @copy will be discarded after copying
#if CPP_STANDARD >= 2011
	UniquePtr& operator = (UniquePtr && copy)
	{
		Switch(copy.Discard());
		return *this;
	}
#endif

	// Assignment operator, retrives raw pointer as an argument
	//    @param ptr - new object's pointer, old one is to be deleted
	UniquePtr& operator = (ElementType* ptr)
	{
		delete pointer;
		pointer = ptr;
		return *this;
	}

	// Assignment operator, template
	//    @param copy - another ut::Ptr object to copy from
	//                  @copy will be discarded after copying
	template<typename _Tp> UniquePtr &
		operator = (UniquePtr<_Tp>& copy)
	{
		Switch(copy.Discard());
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

	// Comparison operator, returns 'true' if @pointer is not a null
	bool operator == (bool b_value) const
	{
		return pointer && b_value;
	}

	// Comparison operator, returns 'true' if @pointer if has the same address as @ptr
	bool operator == (ElementType* ptr) const
	{
		return pointer == ptr;
	}

	// Comparison operator, returns 'false' if @pointer if has the same address as @ptr
	bool operator != (ElementType* ptr) const
	{
		return pointer != ptr;
	}

	// Returns 'true' if both @pointer and @ptr are non-null
	bool operator && (ElementType* ptr) const
	{
		return pointer && ptr;
	}

	// Returns 'true' if both @pointer is not a null and @b_value is 'true'
	bool operator && (bool b_value) const
	{
		return pointer && b_value;
	}

	// Returns 'true' if @pointer is null
	bool operator ! () const
	{
		return !pointer;
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
	void Switch(ElementType* ptr = nullptr)
	{
		if (ptr != pointer)
		{
			delete pointer;
			pointer = ptr;
		}
	}

	// Deletes @pointer object and sets it to null
	void Delete()
	{
		delete pointer;
		pointer = nullptr;
	}

	// Returns pointer to the managed memory. Then @pointer is set to zero,
	// so the managed memory is released without deletion.
	//    @return - pointer to the managed object. Caller is responsible for
	//              managing this object after the call.
	T* Release()
	{
		T* tmp = pointer;
		pointer = nullptr;
		return tmp;
	}

	// Conversion constructor from value via intermediate pointer holder
	UniquePtr(PtrRefCast<ElementType> __ref) : pointer(__ref.ptr)
	{ }

	// Conversion assignment from value via intermediate pointer holder
	UniquePtr& operator = (PtrRefCast<ElementType> __ref)
	{
		if (__ref.ptr != pointer)
		{
			delete pointer;
			pointer = __ref.ptr;
		}
		return *this;
	}

	// These operations convert ut::UniquePtr into and from an ut::PtrRefCast
	// automatically as needed.  This allows constructs such as
	// @code
	//     UniquePtr<Derived>  func_returning_ptr(.....);
	//     ...
	//     UniquePtr<Base> ptr = func_returning_ptr(.....);
	// @endcode
	template<typename _Tp1>
	operator PtrRefCast<_Tp1>()
	{
		return PtrRefCast<_Tp1>(this->Release());
	}

	template<typename _Tp1>
	operator UniquePtr<_Tp1>()
	{
		return UniquePtr<_Tp1>(this->Release());
	}

protected:
	// Managed object, null by default
	ElementType* pointer;
};

// ut::Ptr doesn't support const l-value references for old cpp standards.
#if CPP_STANDARD < 2011
template <typename T> struct LValRef< UniquePtr<T> > { typedef UniquePtr<T>& Type; };
template <typename T> struct RValRef< UniquePtr<T> > { typedef UniquePtr<T>& Type; };
#endif

//----------------------------------------------------------------------------//
// struct ut::ArrPtrRefCast is a proxy structure for ut::Array<ut::UniquePtr>
// convertions. C++03 doesn't support r-value references and move semantics, so
// we need to implement UniquePtr copy constructor with non-reference (but a
// full object) parameter. This is done via intermediate structure ut::ArrPtrRefCast.
template<typename T>
struct ArrPtrRefCast
{
	// Constructor
	explicit ArrPtrRefCast(UniquePtr<T>* p,
	                       size_t n,
	                       size_t r) : arr(p), num(n), rsv(r)
	{ }

	// raw ut::Array members for initialization of the new ut::Array object
	// without actual copy-constructing elements of the array.
	UniquePtr<T>* arr;
	size_t        num;
	size_t        rsv;
};

//----------------------------------------------------------------------------//
// class ut::Array< UniquePtr<T> > is ut::BaseArray template specialization, that
// supports operations with ut::Ptr. It's impossible to use 'unique' smart
// pointer inside usual array container for cpp dialects older than C++11.
// ut::Array<Ptr> implements safe copying and adding unique pointers for arrays.
template<typename T>
class Array< UniquePtr<T> > : public BaseArray< UniquePtr<T> >
{
	typedef UniquePtr<T> ElementType;
	typedef BaseArray< UniquePtr<T> > Base;
public:
	// Default constructor
	Array()
	{ }

	// Constructor, creates @num_elements new empty elements
	//    @param num_elements - how many elements to be initialized
	Array(size_t num_elements) : Base(num_elements)
	{ }

	// Constructor, copies content of another array
	//    @param copy - array to copy
	Array(Array& copy) : Base()
	{
		if (!Base::Resize(copy.GetNum()))
		{
			ThrowError(error::out_of_memory);
		}

		for (size_t i = 0; i < Base::num; i++)
		{
			Base::arr[i] = Move(copy[i]);
		}

		copy.Empty();
	}

	// Constructor, moves content of another array
	//    @param copy - array to copy
#if CPP_STANDARD >= 2011
	Array(Array && copy) : Base(Move(copy))
	{
		copy.arr = nullptr;
		copy.num = 0;
		copy.reserved_elements = 0;
	}
#endif

	// Assignment operator
	//    @param copy - array to copy
	Array& operator = (Array& copy)
	{
		Base::Empty();

		if (!Base::Resize(copy.GetNum()))
		{
			ThrowError(error::out_of_memory);
		}

		for (size_t i = 0; i < Base::num; i++)
		{
			Base::arr[i] = Move(copy[i]);
		}

		copy.Empty();

		return *this;
	}

	// Assignment (move) operator, moves content of another array
	//    @param copy - array to copy
#if CPP_STANDARD >= 2011
	Array& operator = (Array && copy)
	{
		return static_cast<Array&>(Base::operator = (Move(copy)));
	}
#endif

	// Adds new element to the end of the array (r-value reference)
	// uses move semantics to perform copying
	//    @param copy - new element
#if CPP_STANDARD >= 2011
	inline bool Add(ElementType && copy)
	{
		return Base::Add(Move(copy));
	}
#endif // CPP_STANDARD >= 2011

	// Adds new element to the end of the array (reference), @copy is a constant
	//    @param copy - new element
	inline bool Add(typename LValRef<ElementType>::Type copy)
	{
		return Base::Add(copy);
	}

	// Adds new element to the end of the array
	//    @param ptr - pointer to the new element
	bool Add(T* ptr)
	{
		Base::num++;
		if (Base::Realloc())
		{
			new(Base::arr + Base::num - 1) UniquePtr<T>(ptr);
			return true;
		}
		else
		{
			Base::num--;
			return false;
		}
	}

	// Makes copies for every element, nothing is swapped
	//    @param copy - array to copy
	bool Copy(const Array& copy)
	{
		Base::Empty();

		if (!Base::Resize(copy.GetNum()))
		{
			return false;
		}

		for (size_t i = 0; i < Base::num; i++)
		{
			T* new_element = new T(copy[i].GetRef());
			new(Base::arr + i) UniquePtr<T>(new_element);
		}
	}

	// Conversion constructor from value
	Array< ElementType  >(ArrPtrRefCast<T> __ref)
	{
		// just assign members without copying array elements
		Base::arr = __ref.arr;
		Base::num = __ref.num;
		Base::reserved_elements = __ref.rsv;
	}

	// Conversion assignment from value
	Array< ElementType  >& operator = (ArrPtrRefCast<T> __ref)
	{
		if (__ref.arr != Base::arr)
		{
			// destruct current elements
			Base::Empty();
			
			// just assign members without copying array elements
			Base::arr = __ref.arr;
			Base::num = __ref.num;
			Base::reserved_elements = __ref.rsv;
		}
		return *this;
	}

	// This operation converts ut::Array<ut::UniquePtr> into and from an ut::ArrPtrRefCast
	// automatically as needed.
	template<typename _Tp1>
	operator ArrPtrRefCast<_Tp1>()
	{
		// store temporary variables
		UniquePtr<T>* __arr = Base::arr;
		size_t        __num = Base::num;
		size_t        __rsv = Base::reserved_elements;

		// release memory without destruction
		Base::arr = nullptr;
		Base::num = 0;
		Base::reserved_elements = 0;

		// construct ref-caster from the temporary variables
		return ArrPtrRefCast<_Tp1>(__arr, __num, __rsv);
	}

	// This operation converts ut::Array<ut::UniquePtr> into and from value (without copying).
	template<typename _Tp1>
	operator Array< UniquePtr<_Tp1> >()
	{
		// store temporary variables
		UniquePtr<T>* __arr = Base::arr;
		size_t        __num = Base::num;
		size_t        __rsv = Base::reserved_elements;

		// release memory without destruction
		Base::arr = nullptr;
		Base::num = 0;
		Base::reserved_elements = 0;

		// construct new array from the temporary variables
		return Array< UniquePtr<_Tp1>  >(__arr, __num, __rsv);
	}
};

// ut::Array<Ptr> doesn't support const l-value references for old cpp standards.
#if CPP_STANDARD < 2011
template <typename T> struct LValRef< Array< UniquePtr<T> > > { typedef Array< UniquePtr<T> >& Type; };
template <typename T> struct RValRef< Array< UniquePtr<T> > > { typedef Array< UniquePtr<T> >& Type; };
#endif

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//