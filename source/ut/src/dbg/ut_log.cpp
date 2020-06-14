//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "dbg/ut_log.h"
#include "system/ut_console.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Use this global variable for logging, nonetheless you are free to use any
// number of ut::Log instances.
Log log;

//----------------------------------------------------------------------------//
// class ut::Log                                                              //
//----------------------------------------------------------------------------//
// Constructor, does nothing
Log::Log() : console_reflection(false), timestamp(counter)
{}

//----------------------------------------------------------------------------->
// Destructor, closes @file if it's opened
Log::~Log()
{
	ScopeLock sl(mutex);
	if (file.IsOpened())
	{
		file.Close();
	}
}

//----------------------------------------------------------------------------->
// Opens @file for logging
//    @param filename - path to the log file
//    @return - ut::Error if error occurred
Optional<Error> Log::Start(const String& filename, bool console_mirror)
{
	ScopeLock sl(mutex);

	counter.Start();

	if (file.IsOpened())
	{
		return Error(error::already_exists);
	}
	else
	{
		console_reflection = console_mirror;
		return file.Open(filename, file_access_write);
	}
}

//----------------------------------------------------------------------------->
// Closes @file, stops logging
//    @return - ut::Error if error occurred
Optional<Error> Log::End()
{
	ScopeLock sl(mutex);

	counter.Stop();

	if (file.IsOpened())
	{
		console_reflection = false;
		return file.Close();
	}
	else
	{
		return Error(error::empty);
	}
}

// Writes an array of @count elements, each one with a size of @size bytes,
// from the block of memory pointed by @ptr to the current position
//    @param ptr - pointer to the array of elements to be written
//    @param size - size in bytes of each element to be written
//    @param count - number of elements, each one with a size of @size bytes
//    @return - ut::Error if encountered an error
Optional<Error> Log::Write(const void* ptr, size_t size, size_t count)
{
	// bytes written to any stream
	size_t bytes_written = 0;

	// display in console
	if (console_reflection)
	{
		Optional<Error> console_write_error = console.Write(ptr, size, count);
		if (console_write_error)
		{
			return console_write_error;
		}
	}

	// write to file
	ScopeLock sl(mutex);
	if (file.IsOpened())
	{
		// write to the buffer
		Optional<Error> file_write_error = file.Write(ptr, size, count);
		if (file_write_error)
		{
			return file_write_error;
		}

		// and immediately flush
		ut::Optional<ut::Error> flush_error = file.Flush();
		if (flush_error)
		{
			return flush_error;
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Places timestamp and returns a reference to this log object.
Log& Log::Timestamped()
{
	*this << timestamp;
	return *this;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//