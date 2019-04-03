//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Inheritance traits.
namespace inheritance_trait
{
	template <typename B, typename D>
	struct Host
	{
		operator B const volatile * () const;
		operator D const volatile * ();
	};
}

// Helper checker.
template <typename B, typename D>
struct inheritance_helper
{
	template <typename T>
	static compile_time::yes Check(D const volatile *, T);
	static compile_time::no  Check(B const volatile *, int);
};

// Use ut::IsBaseOf<typename B, typename D> to define at compile time if @B is a
// parent type for the class D.
template <typename B, typename D>
struct IsBaseOf
{
	static const bool value =
		sizeof(inheritance_helper<B, D>::Check(inheritance_trait::Host<B, D>(), 0)) == sizeof(compile_time::yes);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//