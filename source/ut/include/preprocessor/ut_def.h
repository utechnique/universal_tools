//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
// True / False macro
#define UT_TRUE 1
#define UT_FALSE 0

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

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//