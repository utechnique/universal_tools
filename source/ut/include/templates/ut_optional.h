//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_def.h"
#include "common/ut_assert.h"
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
	constexpr Optional(const ValueType& v) : value(v), has_value(true)
	{}

	// This constructor moves @v into @value, and sets @has_value to 'true'
	constexpr Optional(ValueType && v) : value(ut::Move(v)), has_value(true)
	{}

	// Copy constructor
	Optional(const Optional& copy) : has_value(copy.has_value)
	{
		if (has_value)
		{
			new (&value)ValueType(copy.value);
		}
	}

	// Move constructor
	Optional(Optional&& other) noexcept : has_value(other.has_value)
	{
		if (has_value)
		{
			new (&value)ValueType(ut::Move(other.value));
		}
		other.Empty();
	}

	// Assignment operator
	Optional& operator = (const Optional& copy)
	{
		if (has_value && copy.has_value) // both objects are already constructed
		{
			value = copy.value;
		}
		else if (copy.has_value) // managed object is uninitialized,
		{                        // but copy is already constructed
			new (&value)ValueType(copy.value);
		}
		else if(has_value) // other object is empty, but managed
		{                  // object is already initialized
			Destruct();
		}

		has_value = copy.has_value;

		return *this;
	}

	// Assignment (move) operator
	Optional& operator = (Optional&& other) noexcept
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

		other.Empty();

		return *this;
	}

	// Assignment operator
	Optional& operator = (const ValueType& v)
	{
		if (has_value)
		{
			value = v;
		}
		else
		{
			new (&value)ValueType(v);
			has_value = true;
		}
		return *this;
	}

	// Assignment (move) operator
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

	// Inheritance operator, provides access to the owned object.
	ValueType* operator -> ()
	{
		return &value;
	}

	// Comparison operator does the same as HasValue() function
	//    @return - 'true' if @has_value == @b
	constexpr bool operator == (bool b) const
	{
		return has_value == b;
	}

	// Function to check if object contains something
	//    @return - 'true' if object owns a value
	constexpr bool HasValue() const
	{
		return has_value;
	}

	// Use this function to get reference to the value (if present)
	//    @return - @value reference
	ValueType& Get()
	{
		UT_ASSERT(has_value);
		return value;
	}

	// Use this function to get const reference to the value (if present)
	//    @return - @value const reference
	constexpr const ValueType& Get() const
	{
		UT_ASSERT(has_value);
		return value;
	}

	// Use this function to move value (if present)
	//    @return - @value r-value reference
	ValueType&& Move()
	{
		UT_ASSERT(has_value);
		return ut::Move(value);
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
		value.~ValueType();
		has_value = false;
	}

	union
	{
		ValueType value;
	};
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
	constexpr Optional(T& t) : Base(&t)
	{}

	// Inheritance operator, provides access to the owned object.
	T* operator -> ()
	{
		return Base::Get();
	}

	// Use this function to get reference to the value (if present)
	//    @return - @value reference
	T& Get()
	{
		return *Base::Get();
	}

	// Use this function to get const reference to the value (if present)
	//    @return - @value const reference
	constexpr const T& Get() const
	{
		return *Base::Get();
	}

	// Use this function to move value (if present)
	//    @return - @value r-value reference
	T&& Move()
	{
		return ut::Move(*Base::Get());
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//