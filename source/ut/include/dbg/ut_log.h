//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "containers/ut_singleton.h"
#include "streams/ut_output_stream.h"
#include "streams/ut_file.h"
#include "system/ut_mutex.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Log is a class for logging events, you can log information using operator
// '<<' so if it was std::cout. Or you can use ut::Log::Print() function.
// ut::Log is thread safe. Object ut::log is created as a singleton, you
// should not create any other instances of the ut::Log class, but don't forget
// to call ut::log.Start() function before using it.
class Log : public OutputStream, public NonCopyable
{
public:
	// Constructor, does nothing
	Log();

	// Destructor, closes @file if it's opened
	~Log();

	// Opens @file for logging
	//    @param filename - path to the log file
	//    @param console_mirror - set 'true' if you want all events
	//                            to be reflected to the console
	//    @return - ut::Error if error occurred
	Optional<Error> Start(const String& filename, bool console_mirror = true);

	// Closes @file, stops logging
	//    @return - ut::Error if error occurred
	Optional<Error> End();

	// Writes an array of @count elements, each one with a size of @size bytes,
	// from the block of memory pointed by @ptr to the current position
	//    @param ptr - pointer to the array of elements to be written
	//    @param size - size in bytes of each element to be written
	//    @param count - number of elements, each one with a size of @size bytes
	//    @return - error if file wasn't opened
	Optional<Error> Write(const void* ptr, size_t size, size_t count);

private:
	// txt file for logging
	File file;

	// used to reflect logging to the console
	bool console_reflection;

	// mutex to provide thread safety
	Mutex mutex;
};

//----------------------------------------------------------------------------//
// Use this global variable for logging, nonetheless you are free to use any
// number of ut::Log instances.
extern Log log;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//