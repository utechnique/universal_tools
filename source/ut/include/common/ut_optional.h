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
public:
	// Default constructor, @has_value is set to 'false'.
	// Use void constructor to return empty ut::Optional object.
	// Example: 'return ut::Optional<ut::Error>();'
	Optional() : value(nullptr), has_value(false)
	{}

	// Copy constructor
	CONSTEXPR Optional(const Optional& copy) : value(copy.has_value ? new T(*copy.value) : nullptr)
	                                         , has_value(copy.has_value)
	{}

	// This constructor copies @t into @value, and sets @has_value to 'true'
	CONSTEXPR Optional(const T& t) : value(new T(t))
	                               , has_value(true)
	{}

#if CPP_STANDARD >= 2011
	// Move constructor
	Optional(Optional && copy) : value(copy.value)
	                           , has_value(copy.has_value)
	{
		copy.value = nullptr;
		copy.has_value = false;
	}

	// Move constructor (value is set)
	CONSTEXPR Optional(T && t) : value(new T(ut::Move(t)))
	                           , has_value(true)
	{}
#endif

	// Assignment operator
	Optional& operator = (const Optional& copy)
	{
		Destruct();
		value = copy.has_value ? new T(*copy.value) : nullptr;
		has_value = copy.has_value;
		return *this;
	}

	// Assignment (move) operator
#if CPP_STANDARD >= 2011
	Optional& operator = (Optional && copy)
	{
		Destruct();
		value = copy.value;
		has_value = copy.has_value;
		copy.value = nullptr;
		copy.has_value = false;
		return *this;
	}
#endif // CPP_STANDARD >= 2011

	// Destructor
	~Optional()
	{
		Destruct();
	}

	// Destructs @value pointer if value is set
	// Also resets @has_value member to false
	void Destruct()
	{
		if (value)
		{
			delete value;
		}
		value = nullptr;
		has_value = false;
	}

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

	// Use this function to get value (if present)
	//    @return - @value reference
	CONSTEXPR const T& Get() const
	{
		UT_ASSERT(value != nullptr);
		return *value;
	}

	// Use this function to move value (if present)
	//    @return - @value r-value reference

	typename ut::RValRef<T>::Type Move()
	{
		UT_ASSERT(value != nullptr);
#if CPP_STANDARD >= 2011
		has_value = false;
#endif
		return ut::Move(*value);
	}

	// Bool conversion operator
	operator bool() const
	{
		return has_value;
	}

private:
	T* value;
	bool has_value;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//