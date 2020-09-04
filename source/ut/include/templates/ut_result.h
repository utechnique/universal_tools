//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_def.h"
#include "templates/ut_optional.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Class ut::Alternate is used as a companion of ut::Result class. Any value
// wrapped with this class is considered alternative (secondary). Use
// ut::MakeAlt() function to construct ut::Alternate object.
template<typename T>
struct Alternate
{
	T value;
};

//----------------------------------------------------------------------------//
// Constructs alternate value from @x
template<typename T>
constexpr Alternate<T> MakeAlt(const T& x)
{
	return { x }; 
}

//----------------------------------------------------------------------------//
// 'Move' version of the MakeAlt<>()
template<typename T>
constexpr Alternate<T> MakeAlt(T&& x)
{
	return { ut::Move(x) };
}

//----------------------------------------------------------------------------//
// Class ut::Result is used for cases where one of two values can be acquired.
// Usually you want to return some value for success case and another value
// for failure. You can set an expected (successful in other words, @R type)
// value using usual constructor, or if you want to set unexpected value
// (failure, @A type) - use ut::Result(ut::MakeAlt<YourType>(your_value)). You
// can get one of two result values calling ut::Result::Get() or
// ut::Result::GetAlt(). Use ut::Result::HasResult() or ut::Result::operator==()
// to figure out what value is kept inside the object.
template<class R, class A>
class Result
{
typedef R ResultType;
typedef A AlternateType;
public:
	// Default constructor, @has_result is true, and result has default value
	Result() : has_result(true)
	{
		new (&result)ResultType();
	}

	// This constructor constructs @R value from @value parameter
	constexpr Result(const ResultType& value) : result(value), has_result(true)
	{}

	// This constructor constructs @R value
	// from @value parameter using move semantics
	constexpr Result(ResultType && value) : result(ut::Move(value)), has_result(true)
	{}

	// This constructor constructs @A value
	constexpr Result(const Alternate<AlternateType>& e) : alt(e.value), has_result(false)
	{}

	constexpr Result(Alternate<AlternateType>&& e) : alt(ut::Move(e.value)), has_result(false)
	{}

	// Copy constructor, uses placement new internally.
	Result(const Result& copy) : has_result(copy.has_result)
	{
		if (has_result)
		{
			new (&result)ResultType(copy.result);
		}
		else
		{
			new (&alt)AlternateType(copy.alt);
		}
	}

	// Move constructor uses placement new and ut::Move() internally.
	// 'constexpr' specifier is used with visual studio only
	// because gcc expects constructor to be empty for 'constexpr'.
	Result(Result&& right) noexcept : has_result(right.has_result)
	{
		if (has_result)
		{
			new (&result)ResultType(ut::Move(right.result));
		}
		else
		{
			new (&alt)AlternateType(ut::Move(right.alt));
		}
	}

	// Assignment operator
	Result& operator = (const Result& copy)
	{
		// destroy existing object
		Destruct();

		// copy status variable
		has_result = copy.has_result;

		// construct new object using copy constructor
		if (has_result)
		{
			new (&result)ResultType(copy.result);
		}
		else
		{
			new (&alt)AlternateType(copy.alt);
		}

		return *this;
	}

	// Move operator
	Result& operator = (Result&& right) noexcept
	{
		// destroy existing object
		Destruct();

		// copy status variable
		has_result = right.has_result;

		// construct new object using move constructor
		if (has_result)
		{
			new (&result)ResultType(ut::Move(right.result));
		}
		else
		{
			new (&alt)AlternateType(ut::Move(right.alt));
		}

		return *this;
	}

	// Destructor is needed to destruct @result or @alt, because cpp 
	// unions do not call destructor. This applies not only to cpp11 (and higher),
	// because custom buffer is used instead of 'union' for older dialects.
	~Result()
	{
		Destruct();
	}

	// Function to check if object contains @R or @A
	//    @return - 'true' if object owns @R value
	constexpr bool HasResult() const
	{
		return has_result;
	}

	// Comparison operator does the same as HasResult() function
	//    @return - 'true' if @has_result == @b
	constexpr bool operator == (bool b) const
	{
		return has_result == b;
	}

	// Use this function to get @R value (if present)
	//    @return - @result reference
	constexpr const ResultType& Get() const
	{
		return result;
	}

	// Use this function to get @R value (if present)
	//    @return - @result reference
	ResultType& Get()
	{
		return result;
	}

	// Use this function to move @R value (if present)
	//    @return - @result r-value reference
	ResultType&& Move() noexcept
	{
		return ut::Move(result);
	}

	// Use this function to move @R value if it's present or throw
	// @A otherwise.
	//    @return - @result r-value reference
	ResultType&& MoveOrThrow()
	{
		if (!has_result)
		{
			throw alt;
		}
		return ut::Move(result);
	}

	// Use this function to get @A value (if present)
	//    @return - @alt reference
	constexpr const AlternateType& GetAlt() const
	{
		return alt;
	}

	// Use this function to get @A value (if present)
	//    @return - @alt reference
	AlternateType& GetAlt()
	{
		return alt;
	}

	// Use this function to move @A value (if present)
	//    @return - @alt r-value reference
	AlternateType&& MoveAlt()
	{
		return ut::Move(alt);
	}

	// Bool conversion operator
	operator bool() const
	{
		return has_result;
	}

private:
	// Destructs managed object
	inline void Destruct()
	{
		if (has_result)
		{
			result.~ResultType();
		}
		else
		{
			alt.~AlternateType();
		}
	}

	union
	{
		ResultType result;
		AlternateType alt;
	};
	bool has_result;
};

//----------------------------------------------------------------------------//
// Specialization for void type.
template<class A>
class Result<void, A> : private Optional<A>
{
	typedef A AlternateType;
	typedef Optional<AlternateType> Base;
public:
	// Default constructor
	constexpr Result() : Base()
	{}

	// Constructs alternative value
	constexpr Result(const Alternate<AlternateType>& a) : Base(a.value)
	{}

	// Constructor moves alternative value
	constexpr Result(Alternate<AlternateType>&& a) : Base(ut::Move(a.value))
	{}

	// Copy constructor
	constexpr Result(const Result& copy) : Base(copy)
	{}

	// Move constructor
	constexpr Result(Result&& rval) noexcept : Base(ut::Move(rval))
	{}

	// Function to check if object was constructed with void type or @A
	constexpr bool HasResult() const
	{
		return !Base::HasValue();
	}

	// Comparison operator does the same as HasResult() function
	//    @return - 'true' if @has_result == @b
	constexpr bool operator == (bool b) const
	{
		return Base::HasValue() != b;
	}

	// Bool conversion operator
	constexpr operator bool() const
	{
		return !Base::HasValue();
	}

	// Use this function to get @A value (if present)
	//    @return - @alt reference
	constexpr const AlternateType& GetAlt() const
	{
		return Base::Get();
	}

	// Use this function to move @A value (if present)
	//    @return - @alt r-value reference
	AlternateType&& MoveAlt()
	{
		return Base::Move();
	}
};

//----------------------------------------------------------------------------//
// Specialization for the case when 'Result' type is a reference type.
template<class R, class A>
class Result<R&, A> : public Result<R*, A>
{
	typedef R ResultType;
	typedef A AlternateType;
	typedef Result<ResultType*, AlternateType> Base;
public:
	// This constructor constructs binds result reference
	constexpr Result(ResultType& result_ref) : Base(&result_ref)
	{}

	// This constructor constructs @A value
	constexpr Result(const Alternate<AlternateType>& e) : Base(e)
	{}

	constexpr Result(Alternate<AlternateType>&& e) : Base(ut::Move(e))
	{}

	// Use this function to get @R value (if present)
	//    @return - @result reference
	constexpr ResultType& Get() const
	{
		return *Base::Get();
	}

	// Use this function to move @R value (if present)
	//    @return - @result r-value reference
	ResultType&& Move()
	{
		return ut::Move(*Base::Move());
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//