//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// First element of the ut::Pair.
template <typename T>
class PairFirst
{
public:
	PairFirst() {}
	PairFirst(const T& value) : first(value) {}
	PairFirst(T&& value) : first(ut::Move(value)) {}
	PairFirst(const PairFirst&) = default;
	PairFirst(PairFirst&&) noexcept = default;
	PairFirst& operator = (const PairFirst&) = default;
	PairFirst& operator = (PairFirst&&) noexcept = default;
	const T& GetFirst() const { return first; }
	T first;
};

// Specialization of the first element of the ut::Pair for the constant types.
template <typename T>
class PairFirst<const T>
{
public:
	PairFirst() {}
	PairFirst(const T& value) : first(value) {}
	PairFirst(T&& value) : first(ut::Move(value)) {}
	PairFirst(const PairFirst&) = default;
	PairFirst(PairFirst&&) noexcept = default;
	PairFirst& operator = (const PairFirst&) = default;
	PairFirst& operator = (PairFirst&&) noexcept = default;
	const T& GetFirst() const { return first; }
private:
	T first;
};

// Second element of the ut::Pair.
template <typename T>
class PairSecond
{
public:
	PairSecond() {}
	PairSecond(const T& value) : second(value) {}
	PairSecond(T&& value) : second(ut::Move(value)) {}
	PairSecond(const PairSecond&) = default;
	PairSecond(PairSecond&&) noexcept = default;
	PairSecond& operator = (const PairSecond&) = default;
	PairSecond& operator = (PairSecond&&) noexcept = default;
	const T& GetSecond() const { return second; }
	T second;
};

// Specialization of the second element of the ut::Pair for the constant types.
template <typename T>
class PairSecond<const T>
{
public:
	PairSecond() {}
	PairSecond(const T& value) : second(value) {}
	PairSecond(T&& value) : second(ut::Move(value)) {}
	PairSecond(const PairSecond&) = default;
	PairSecond(PairSecond&&) noexcept = default;
	PairSecond& operator = (const PairSecond&) = default;
	PairSecond& operator = (PairSecond&&) noexcept = default;
	const T& GetSecond() const { return second; }
private:
	T second;
};

//----------------------------------------------------------------------------//
// ut::Pair is a struct template that provides a way to store two
// heterogeneous objects as a single unit.
template <typename T1, typename T2>
class Pair : public PairFirst<T1>, public PairSecond<T2>
{
	typedef PairFirst<T1> First;
	typedef PairSecond<T2> Second;
public:
	// Default constructor
	Pair()
	{}

	// Constructor, both arguments are const references
	Pair(const T1& t1, const T2& t2) : First(t1), Second(t2)
	{}

	// Constructor, both arguments are r-value references
	Pair(T1&& t1, T2&& t2) : First(Move(t1)), Second(Move(t2))
	{}

	// Constructor, first argument is l-value reference,
	// second one is r-value reference
	Pair(const T1& t1, T2&& t2) : First(t1), Second(Move(t2))
	{}

	// Constructor, first argument is r-value reference,
	// second one is l-value reference
	Pair(T1&& t1, const T2& t2) : First(Move(t1)), Second(t2)
	{}

	// Copy and assignment
	Pair(const Pair&) = default;
	Pair(Pair&&) noexcept = default;
	Pair& operator = (const Pair&) = default;
	Pair& operator = (Pair&&) noexcept = default;
};

//----------------------------------------------------------------------------//
// Specialization for the case when the first type is a reference.
template <typename T1, typename T2>
class Pair<T1&, T2> : public PairSecond<T2>
{
	typedef PairSecond<T2> Second;
public:
	// Constructor, both arguments are references
	Pair(T1& t1, const T2& t2) : first(t1), Second(t2)
	{}

	// Constructor, second argument is r-value reference
	Pair(T1& t1, T2&& t2) : first(t1), Second(Move(t2))
	{}

	// Copy constructor
	Pair(const Pair& copy) : first(copy.first), Second(copy.second)
	{}

	// Move constructor
	Pair(Pair&& copy) noexcept : first(copy.first), Second(Move(copy.second))
	{}

	// reference
	T1& first;

private:
	// Assignment operators are prohibited.
	Pair& operator = (const Pair&) = delete;
	Pair& operator = (Pair&&) = delete;
};

//----------------------------------------------------------------------------//
// Specialization for the case when the second type is a reference.
template <typename T1, typename T2>
class Pair<T1, T2&> : public PairFirst<T1>
{
	typedef PairFirst<T1> First;
public:
	// Constructor, both arguments are references
	Pair(const T1& t1, T2& t2) : First(t1), second(t2)
	{}

	// Constructor, first argument is r-value reference
	Pair(T1&& t1, T2& t2) : First(Move(t1)), second(t2)
	{}

	// Copy constructor
	Pair(const Pair& copy) : First(copy.first), second(copy.second)
	{}

	// Move constructor
	Pair(Pair && copy) noexcept : First(Move(copy.first)), second(copy.second)
	{}

	// reference
	T2& second;

private:
	// Assignment operators are prohibited.
	Pair& operator = (const Pair&) = delete;
	Pair& operator = (Pair&&) = delete;
};

//----------------------------------------------------------------------------//
// Specialization for the case when both types are references.
template <typename T1, typename T2>
class Pair<T1&, T2&>
{
public:
	// Constructor, both arguments are references
	Pair(T1& t1, T2& t2) : first(t1), second(t2)
	{}

	// Copy constructor
	Pair(const Pair& copy) : first(copy.first), second(copy.second)
	{}

	// references
	T1& first;
	T2& second;

private:
	// Assignment operators are prohibited.
	Pair& operator = (const Pair&) = delete;
	Pair& operator = (Pair&&) = delete;
};

//----------------------------------------------------------------------------//
// Specialize type name function for the map type.
template <typename Type1, typename Type2> struct Type< Pair<Type1, Type2> >
{
	static inline const char* Name() { return "pair"; }
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//