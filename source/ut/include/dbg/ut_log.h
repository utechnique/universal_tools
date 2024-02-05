//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "text/ut_string.h"
#include "templates/ut_singleton.h"
#include "streams/ut_output_stream.h"
#include "streams/ut_file.h"
#include "system/ut_mutex.h"
#include "system/ut_time.h"
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

	// Log can't be moved
	Log(Log&&) = delete;
	Log& operator = (Log&&) = delete;

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

	// Places timestamp and returns a reference to this log object.
	Log& Timestamped();

	// Const reference to the time counter. Use this counter to place
	// a timestamp in the output '<<' operator sequence.
	const time::Counter& timestamp;

private:
	// txt file for logging
	File file;

	// timer to record time of the event
	time::Counter counter;

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