//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//

// True / False macro
#define UT_TRUE 1
#define UT_FALSE 0

// C++ standards
#ifndef CPP_STANDARD
	#define CPP_STANDARD 2014
#endif

// debug macros
#ifdef NDEBUG
	#define UT_DEBUG 0
#else
	#ifdef DEBUG
		#define UT_DEBUG 1
	#else
		#define UT_DEBUG 0
	#endif
#endif

// Enable to detect memory leaks
#define UT_MEMORY_PROFILING UT_DEBUG

// Enable to use Visual Leak Detector in visual studio
#define UT_USE_VLD 0

// General namespace macro
#define START_NAMESPACE(__namespace) namespace __namespace {
#define END_NAMESPACE(__namespace) }

// On compilers which don't allow in-class initialization of static integral
// constant members, we must use enums as a workaround if we want the constants
// to be available at compile-time. This macro gives us a convenient way to
// declare such constants.
#ifdef UT_NO_INCLASS_MEMBER_INITIALIZATION
#	define UT_INCLASS_STATIC_CONSTANT(__type, __assignment) enum { __assignment }
#else
#	define UT_INCLASS_STATIC_CONSTANT(__type, __assignment) static const __type __assignment
#endif

// Use this macro to inform UT that current class has no default constructor.
// Lower cpp versions (before C++11) do not support IsDefaultConstructible
// trait natively. So UT expects some help from you. Place this macro in the
// 'public:' section inside your class declaration. Example:
//
// class MyClass
// {
// public:
//     UT_NO_DEFAULT_CONSTRUCTOR
// }
#define UT_NO_DEFAULT_CONSTRUCTOR void MarkAsNonConstructible();

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//