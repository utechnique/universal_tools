//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_compile_time.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Use ut::AddPointer template to get a pointer type to the specified object.
template <typename T> struct AddPointer { typedef T* Type; };

// Use ut::RemovePointer template to get a type with no pointer of
// the specified object.
template <typename T> struct RemovePointer;
template <typename T> struct RemovePointer<T*> { typedef T Type; };

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//