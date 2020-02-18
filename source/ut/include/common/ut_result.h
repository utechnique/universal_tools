//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_def.h"
#include "ut_cpp11.h"
#include "ut_optional.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Class ut::Alternate is used as a companion of ut::Result class. Any value
// wrapped with this class is considered alternative (secondary). Use
// ut::MakeAlt() function to construct ut::Alternate object.
template<typename T>
struct Alternate
{
#if CPP_STANDARD < 2011
	Alternate() {}
	Alternate(const T& v) : value(v) {}
#endif
	T value;
};

//----------------------------------------------------------------------------//
// Constructs alternate value from @x
template<typename T>
CONSTEXPR Alternate<T> MakeAlt(const T& x)
{
#if CPP_STANDARD >= 2011
	return { x }; 
#else
	return Alternate<T>(x);
#endif
}

//----------------------------------------------------------------------------//
// 'Move' version of the MakeAlt<>()
#if CPP_STANDARD >= 2011
template<typename T>
constexpr Alternate<T> MakeAlt(T && x)
{
	return { Move(x) };
}
#endif // CPP_STANDARD >= 2011

//----------------------------------------------------------------------------//
// Class ut::Result is used for cases where one of two values can be acquired.
// Usually you want to return some value for success case and another value
// for failure. You can set an expected (successful in other words, @R type)
// value using usual constructor, or if you want to set unexpected value
// (failure, @A type) - use ut::Result(ut::MakeAlt<YourType>(your_value)). You
// can get one of two result values calling ut::Result::GetResult() or
// ut::Result::GetAlt(). Use ut::Result::HasResult() or ut::Result::operator==()
// to figure out what value is kept inside the object.
template<class R, class A>
class Result
{
typedef R ResultType;
typedef A AlternateType;
public:
	// This constructor constructs @R value from @value parameter
#if CPP_STANDARD >= 2011
	CONSTEXPR Result(const ResultType& value) : result(value), has_result(true)
	{}
#endif

	// This constructor constructs @R value from @value parameter
	// Implementation for older cpp dialects
#if CPP_STANDARD < 2011
	Result(typename LValRef<ResultType>::Type value) : has_result(true)
	{
		new (union_buffer)ResultType(value);
	}
#endif

	// This constructor constructs @R value
	// from @value parameter using move semantics
#if CPP_STANDARD >= 2011
	CONSTEXPR Result(ResultType && value) : result(Move(value)), has_result(true)
	{}
#endif

	// This constructor constructs @A value
#if CPP_STANDARD >= 2011
	CONSTEXPR Result(const Alternate<AlternateType>& e) : alt(e.value), has_result(false)
	{}

	CONSTEXPR Result(Alternate<AlternateType>&& e) : alt(Move(e.value)), has_result(false)
	{}
#endif

	// This constructor constructs @A value
	// Implementation for older cpp dialects
#if CPP_STANDARD < 2011
	Result(const Alternate<AlternateType>& e) : has_result(false)
	{
		new (union_buffer)AlternateType(e.value);
	}
#endif

	// Copy constructor, uses placement new internally.
	// 'constexpr' specifier is used with visual studio only
	// because gcc expects constructor to be empty for 'constexpr'.
	WIN_CONSTEXPR Result(const Result& copy) : has_result(copy.has_result)
	{
		if (has_result)
		{
			#if CPP_STANDARD >= 2011
				new (&result)ResultType(copy.result);
			#else
				new (union_buffer)ResultType(*((R*)copy.union_buffer));
			#endif
		}
		else
		{
			#if CPP_STANDARD >= 2011
				new (&alt)AlternateType(copy.alt);
			#else
				new (union_buffer)AlternateType(*((A*)copy.union_buffer));
			#endif
		}
	}

	// Move constructor uses placement new and ut::Move() internally.
	// 'constexpr' specifier is used with visual studio only
	// because gcc expects constructor to be empty for 'constexpr'.
#if CPP_STANDARD >= 2011
	Result(Result&& right) : has_result(right.has_result)
	{
		if (has_result)
		{
			new (&result)ResultType(Move(right.result));
		}
		else
		{
			new (&alt)AlternateType(Move(right.alt));
		}
	}
#endif

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
			#if CPP_STANDARD >= 2011
				new (&result)ResultType(copy.result);
			#else
				new (union_buffer)ResultType(*((R*)copy.union_buffer));
			#endif
		}
		else
		{
			#if CPP_STANDARD >= 2011
				new (&alt)AlternateType(copy.alt);
			#else
				new (union_buffer)AlternateType(*((A*)copy.union_buffer));
			#endif
		}

		return *this;
	}

	// Move operator
#if CPP_STANDARD >= 2011
	Result& operator = (Result&& right)
	{
		// destroy existing object
		Destruct();

		// copy status variable
		has_result = right.has_result;

		// construct new object using move constructor
		if (has_result)
		{
			new (&result)ResultType(Move(right.result));
		}
		else
		{
			new (&alt)AlternateType(Move(right.alt));
		}

		return *this;
	}
#endif

	// Destructor is needed to destruct @result or @alt, because cpp 
	// unions do not call destructor. This applies not only to cpp11 (and higher),
	// because custom buffer is used instead of 'union' for older dialects.
	~Result()
	{
		Destruct();
	}

	// Function to check if object contains @R or @A
	//    @return - 'true' if object owns @R value
	CONSTEXPR bool HasResult() const
	{
		return has_result;
	}

	// Comparison operator does the same as HasResult() function
	//    @return - 'true' if @has_result == @b
	CONSTEXPR bool operator == (bool b) const
	{
		return has_result == b;
	}

	// Use this function to get @R value (if present)
	//    @return - @result reference
	CONSTEXPR const ResultType& GetResult() const
	{
		#if CPP_STANDARD >= 2011
			return result;
		#else
			return *((ResultType*)union_buffer);
		#endif
	}

	// Use this function to move @R value (if present)
	//    @return - @result r-value reference
	typename RValRef<ResultType>::Type MoveResult()
	{
		#if CPP_STANDARD >= 2011
			return Move(result);
		#else
			// 'Move' stub for older cpp dialects
			return *((ResultType*)union_buffer);
		#endif
	}

	// Use this function to get @A value (if present)
	//    @return - @alt reference
	CONSTEXPR const AlternateType& GetAlt() const
	{
		#if CPP_STANDARD >= 2011
			return alt;
		#else
			return *((AlternateType*)union_buffer);
		#endif
	}

	// Use this function to move @A value (if present)
	//    @return - @alt r-value reference
	typename RValRef<AlternateType>::Type MoveAlt()
	{
		#if CPP_STANDARD >= 2011
			return Move(alt);
		#else
			// 'Move' stub for older cpp dialects
			return *((AlternateType*)union_buffer);
		#endif
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
			#if CPP_STANDARD >= 2011
				result.~ResultType();
			#else
				ResultType* result = (ResultType*)union_buffer;
				result->~ResultType();
			#endif
		}
		else
		{
			#if CPP_STANDARD >= 2011
				alt.~AlternateType();
			#else
				AlternateType* alt = (AlternateType*)union_buffer;
				alt->~AlternateType();
			#endif
		}
	}

#if CPP_STANDARD >= 2011
	union
	{
		ResultType result;
		AlternateType alt;
	};
#else
	// C98 unions don't support members with non-default copy constructor
	uint8 union_buffer[sizeof(R) > sizeof(A) ? sizeof(R) : sizeof(A)];
#endif
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
	CONSTEXPR Result() : Base()
	{}

	// Constructs alternative value
	CONSTEXPR Result(const Alternate<AlternateType>& a) : Base(a.value)
	{}

	// Constructor moves alternative value
#if CPP_STANDARD >= 2011
	CONSTEXPR Result(Alternate<AlternateType>&& a) : Base(ut::Move(a.value))
	{}
#endif

	// Copy constructor
	CONSTEXPR Result(const Result& copy) : Base(copy)
	{}

	// Move constructor
#if CPP_STANDARD >= 2011
	CONSTEXPR Result(Result&& rval) : Base(ut::Move(rval))
	{}
#endif

	// Function to check if object was constructed with void type or @A
	CONSTEXPR bool HasResult() const
	{
		return !Base::HasValue();
	}

	// Comparison operator does the same as HasResult() function
	//    @return - 'true' if @has_result == @b
	CONSTEXPR bool operator == (bool b) const
	{
		return Base::HasValue() != b;
	}

	// Bool conversion operator
	CONSTEXPR operator bool() const
	{
		return !Base::HasValue();
	}

	// Use this function to get @A value (if present)
	//    @return - @alt reference
	CONSTEXPR const AlternateType& GetAlt() const
	{
		return Base::Get();
	}

	// Use this function to move @A value (if present)
	//    @return - @alt r-value reference
	typename RValRef<AlternateType>::Type MoveAlt()
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
	CONSTEXPR Result(ResultType& result_ref) : Base(&result_ref)
	{}

	// This constructor constructs @A value
	CONSTEXPR Result(const Alternate<AlternateType>& e) : Base(e)
	{}

#if CPP_STANDARD >= 2011
	CONSTEXPR Result(Alternate<AlternateType>&& e) : Base(Move(e))
	{}
#endif

	// Use this function to get @R value (if present)
	//    @return - @result reference
	CONSTEXPR ResultType& GetResult() const
	{
		return *Base::GetResult();
	}

	// Use this function to move @R value (if present)
	//    @return - @result r-value reference
	typename RValRef<ResultType>::Type MoveResult()
	{
		return Move(*Base::MoveResult());
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//