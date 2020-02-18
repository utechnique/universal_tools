//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_def.h"
#include "ut_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
#if CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
#define CONSTEXPR constexpr
#define PROHIBITED = delete
#define THROWS_EXCEPTION noexcept(false)

// Some objects can have 'constexpr' specifier only for windows platform
#if UT_WINDOWS
	#define WIN_CONSTEXPR constexpr
#else
	#define WIN_CONSTEXPR
#endif

// R-Value reference type
template <typename T> struct RValRef { typedef T&& Type; };

// L-Value reference type
template <typename T> struct LValRef { typedef const T& Type; };

// RemoveReference<type> will remove any references from a type.
template <typename T> struct RemoveReference { typedef T Type; };
template <typename T> struct RemoveReference<T& > { typedef T Type; };
template <typename T> struct RemoveReference<T&&> { typedef T Type; };

// Casts a reference to an rvalue reference.
template <typename T>
FORCEINLINE typename RemoveReference<T>::Type&& Move(T&& ref)
{
	return (typename RemoveReference<T>::Type&&)ref;
}

// Casts a reference to an lvalue reference.
template <typename T>
FORCEINLINE constexpr T&& Forward(typename RemoveReference<T>::Type& ref)
{
	return static_cast<T&&>(ref);
}

// Casts a reference to an rvalue reference.
template <typename T>
FORCEINLINE constexpr T&& Forward(typename RemoveReference<T>::Type&& ref)
{
	return static_cast<T&&>(ref);
}

//----------------------------------------------------------------------------//
#else

// nullptr
#define nullptr NULL

// RemoveReference<type> will remove any references from a type.
template <typename T> struct RemoveReference { typedef T Type; };
template <typename T> struct RemoveReference<T& > { typedef T Type; };

// ut::RefConstness<T> determines whether ut::LValRef<T>
// will have const or non-const reference type.
#define UT_SET_CONSTNESS(__value) { static const bool is_const = __value; };
template<typename T> struct RefConstness UT_SET_CONSTNESS(true)

// ut::LValRefCreator holds const or non-const reference type according to the second
// template argument - type will be non-const if @is_const is 'false'. 
template<typename T, bool is_const> struct LValRefCreator { typedef const T& Type; };
template<typename T> struct LValRefCreator<T, false> { typedef T& Type; };

// L-Value reference type
template <typename T> struct LValRef
{
	typedef typename LValRefCreator<T, RefConstness<T>::is_const>::Type Type;
};
template <typename T> struct LValRef<T&> { typedef T& Type; };

// R-Value reference type
template <typename T> struct RValRef { typedef typename LValRef<T>::Type Type; };

// Casts a reference to an rvalue reference.
template <typename T>
FORCEINLINE typename RemoveReference<T>::Type& Move(T& ref)
{
	return static_cast<typename RemoveReference<T>::Type&>(ref);
}

// Casts a reference to an lvalue reference.
template <typename T>
FORCEINLINE typename LValRef<T>::Type Forward(typename LValRef<T>::Type ref)
{
	return ref;
}

#define CONSTEXPR
#define WIN_CONSTEXPR
#define PROHIBITED
#define THROWS_EXCEPTION

#endif // CPP_STANDARD >= 2011
//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//