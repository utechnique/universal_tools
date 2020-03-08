//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_def.h"
#include "ut_assert.h"
#include "ut_cpp11.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Optional class can logically contain or not contain @value of the type @T
// Use ut::Optional::operator==() or ut::Optional::HasValue() to figure
// out if object contains a value. Use ut::Optional::Get() to get that value.
// This class is usually used for errors and optional values.
template<typename T>
class Optional
{
	typedef T ValueType;
public:
	// Default constructor, @has_value is set to 'false'.
	// Use void constructor to return empty ut::Optional object.
	// Example: 'return ut::Optional<ut::Error>();'
	Optional() : has_value(false)
	{}

	// This constructor copies @v into @value, and sets @has_value to 'true'
#if CPP_STANDARD >= 2011
	CONSTEXPR Optional(const ValueType& v) : value(v), has_value(true)
	{}
#else
	Optional(typename LValRef<ValueType>::Type v) : has_value(true)
	{
		new (union_buffer)ValueType(v);
	}
#endif

	// This constructor moves @v into @value, and sets @has_value to 'true'
#if CPP_STANDARD >= 2011
	CONSTEXPR Optional(ValueType && v) : value(ut::Move(v)), has_value(true)
	{}
#endif

	// Copy constructor
	Optional(typename LValRef<Optional>::Type copy) : has_value(copy.has_value)
	{
		if (has_value)
		{
#if CPP_STANDARD >= 2011
			new (&value)ValueType(copy.value);
#else
		
			new (union_buffer)ValueType(copy.Get());
#endif
		}
	}

	// Move constructor
#if CPP_STANDARD >= 2011
	Optional(Optional && copy) : has_value(copy.has_value)
	{
		if (has_value)
		{
			new (&value)ValueType(ut::Move(copy.value));
		}
	}
#endif

	// Assignment operator
	Optional& operator = (typename LValRef<Optional>::Type copy)
	{
		if (has_value && copy.has_value) // both objects are already constructed
		{
#if CPP_STANDARD >= 2011
			value = copy.value;
#else
			Get() = copy.Get();
#endif
		}
		else if (copy.has_value) // managed object is uninitialized,
		{                        // but copy is already constructed
#if CPP_STANDARD >= 2011
			new (&value)ValueType(copy.value);
#else
			new (union_buffer)ValueType(copy.Get());
#endif
		}
		else if(has_value) // other object is empty, but managed
		{                  // object is already initialized
			Destruct();
		}

		has_value = copy.has_value;

		return *this;
	}

	// Assignment (move) operator
#if CPP_STANDARD >= 2011
	Optional& operator = (Optional && other)
	{
		if (has_value && other.has_value) // both objects are already constructed
		{
			value = ut::Move(other.value);
		}
		else if (other.has_value) // managed object is uninitialized,
		{                         // but other object is already constructed
			new (&value)ValueType(ut::Move(other.value));
		}
		else if(has_value) // other object is empty, but managed
		{                  // object is already initialized
			Destruct();
		}

		has_value = other.has_value;

		return *this;
	}
#endif // CPP_STANDARD >= 2011

	// Assignment operator
	Optional& operator = (typename LValRef<ValueType>::Type v)
	{
		if (has_value)
		{
#if CPP_STANDARD >= 2011
			value = v;
#else
			Get() = v;
#endif
		}
		else
		{
#if CPP_STANDARD >= 2011
			new (&value)ValueType(v);
#else
			new (union_buffer)ValueType(v);
#endif
			has_value = true;
		}
		return *this;
	}

	// Assignment (move) operator
#if CPP_STANDARD >= 2011
	Optional& operator = (ValueType&& v)
	{
		if (has_value)
		{
			value = ut::Move(v);
		}
		else
		{
			new (&value)ValueType(ut::Move(v));
			has_value = true;
		}
		return *this;
	}
#endif // CPP_STANDARD >= 2011

	// Function to check if object contains something
	//    @return - 'true' if object owns a value
	CONSTEXPR bool HasValue() const
	{
		return has_value;
	}

	// Comparison operator does the same as HasValue() function
	//    @return - 'true' if @has_value == @b
	CONSTEXPR bool operator == (bool b) const
	{
		return has_value == b;
	}

	// Use this function to get reference to the value (if present)
	//    @return - @value reference
	ValueType& Get()
	{
		UT_ASSERT(has_value);
#if CPP_STANDARD >= 2011
		return value;
#else
		return *reinterpret_cast<ValueType*>(union_buffer);
#endif
	}

	// Use this function to get const reference to the value (if present)
	//    @return - @value const reference
	CONSTEXPR const ValueType& Get() const
	{
		UT_ASSERT(has_value);
#if CPP_STANDARD >= 2011
		return value;
#else
		return *reinterpret_cast<const ValueType*>(union_buffer);
#endif
	}

	// Use this function to move value (if present)
	//    @return - @value r-value reference
	typename ut::RValRef<ValueType>::Type Move()
	{
		UT_ASSERT(has_value);
#if CPP_STANDARD >= 2011
		return ut::Move(value);
#else
		return Get();
#endif
	}

	// Destroyes current value if it's present.
	void Empty()
	{
		if (has_value)
		{
			Destruct();
		}
	}

	// Bool conversion operator
	operator bool() const
	{
		return has_value;
	}

	// Destructor
	~Optional()
	{
		Empty();
	}

private:
	// Destructs @value pointer if value is set
	// Also resets @has_value member to false
	inline void Destruct()
	{
#if CPP_STANDARD >= 2011
		value.~ValueType();
#else
		Get().~ValueType();
#endif
		has_value = false;
	}

#if CPP_STANDARD >= 2011
	union
	{
		ValueType value;
	};
#else
	// C98 unions don't support members with non-default copy constructor
	byte union_buffer[sizeof(ValueType)];
#endif
	bool has_value;
};

//----------------------------------------------------------------------------//
// Specialization for reference types.
template<typename T>
class Optional<T&> : public Optional<T*>
{
	typedef Optional<T*> Base;
public:
	// Default constructor, @has_value is set to 'false'.
	Optional() : Base()
	{}

	// This constructor copies @t into @value, and sets @has_value to 'true'
	CONSTEXPR Optional(T& t) : Base(&t)
	{}

	// Use this function to get reference to the value (if present)
	//    @return - @value reference
	T& Get()
	{
		return *Base::Get();
	}

	// Use this function to get const reference to the value (if present)
	//    @return - @value const reference
	CONSTEXPR const T& Get() const
	{
		return *Base::Get();
	}

	// Use this function to move value (if present)
	//    @return - @value r-value reference
	typename ut::RValRef<T>::Type Move()
	{
		return ut::Move(*Base::Get());
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//