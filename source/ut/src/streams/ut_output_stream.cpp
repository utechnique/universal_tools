//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "streams/ut_output_stream.h"
#include "system/ut_lock.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Constructor.
OutputStream::Locked::Locked(OutputStream& output_stream,
                             Mutex& insertion_mutex) : stream(output_stream)
		                                             , mutex(insertion_mutex)
{
	mutex.Lock();
}

// Destructor.
OutputStream::Locked::~Locked()
{
	mutex.Unlock();
}

// Synchronizes the associated stream buffer with its controlled output sequence.
//    @return - error code if failed
Optional<Error> OutputStream::Flush()
{
	return Error(error::not_supported);
}

// Returns stream offset to the current cursor position (in bytes)
//    @return - cursor position if file is opened, or error otherwise
Result<stream::Cursor, Error> OutputStream::GetCursor() const
{
	return MakeError(error::not_supported);
}

// Sets file offset to the current cursor position (in bytes)
//    @param offset - offset in bytes from @origin
//    @param origin - offset from the beginning of the stream
//                    @offset will be added to this parameter
//    @return - error code if failed
Optional<Error> OutputStream::MoveCursor(stream::Cursor offset,
                                         stream::Position origin)
{
	return Error(error::not_supported);
}

// Returns size of the stream buffer or error if failed,
// returns "not_implemented" error if not overridend by child class
Result<size_t, Error> OutputStream::GetSize()
{
	return MakeError(error::not_implemented);
}

// Locks '<<' insertion sequence.
OutputStream::Locked OutputStream::Lock()
{
	return Locked(*this, insertion_mutex);
}

// Operator '<<' applied to an output stream is known as insertion operator.
// Use it for formatted human-readable text data output.
OutputStream& OutputStream::operator << (OutputStream& stream)
{
	return (*this);
}

OutputStream& OutputStream::operator << (bool val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (int16 val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (uint16 val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (int32 val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (uint32 val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (int64 val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (uint64 val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (float val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (double val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (long double val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (void* val)
{
	return OutputStream::operator << (Print(val));
}

OutputStream& OutputStream::operator << (const char* str)
{
	String ut_str(str);
	return OutputStream::operator << (ut_str);
}

OutputStream& OutputStream::operator << (const String& str)
{
	String formatted_string(str);
	formatted_string.FixCarriageReturn();
	Optional<Error> write_error = Write(formatted_string.ToCStr(),
	                                    1,
	                                    formatted_string.Length());
	if (write_error)
	{
		throw Error(write_error.Move());
	}
	return *this;
}

OutputStream& OutputStream::operator << (const time::Counter& timestamp)
{
	ut::uint64 ns = timestamp.GetTime<time::nanoseconds, ut::uint64>();

	ut::uint64 days = time::Convert<time::nanoseconds, time::days>(ns);
	ns -= time::Convert<time::days, time::nanoseconds>(days);

	ut::uint64 hours = time::Convert<time::nanoseconds, time::hours>(ns);
	ns -= time::Convert<time::hours, time::nanoseconds>(hours);

	ut::uint64 minutes = time::Convert<time::nanoseconds, time::minutes>(ns);
	ns -= time::Convert<time::minutes, time::nanoseconds>(minutes);

	ut::uint64 seconds = time::Convert<time::nanoseconds, time::seconds>(ns);
	ns -= time::Convert<time::seconds, time::nanoseconds>(seconds);

	ut::uint64 milliseconds = time::Convert<time::nanoseconds, time::milliseconds>(ns);
	ns -= time::Convert<time::milliseconds, time::nanoseconds>(milliseconds);

	*this << "[";
	if (days != 0)
	{
		*this << days << "d ";
	}

	return *this << hours << ":" << minutes << ":" << seconds << ":" << milliseconds << "]";
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//