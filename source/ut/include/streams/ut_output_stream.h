//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
#include "streams/ut_base_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::OutputStream is an abstract parent class for output streams.
class OutputStream : public BaseStream
{
public:
	// Writes an array of @count elements, each one with a size of @size bytes,
	// from the block of memory pointed by @ptr to the current position
	//    @param ptr - pointer to the array of elements to be written
	//    @param size - size in bytes of each element to be written
	//    @param count - number of elements, each one with a size of @size bytes
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Write(const void* ptr, size_t size, size_t count) = 0;

	// Synchronizes the associated stream buffer with its controlled output sequence.
	//    @return - error code if failed
	virtual Optional<Error> Flush();

	// Returns stream offset to the current cursor position (in bytes)
	//    @return - cursor position if file is opened, or error otherwise
	virtual Result<stream::Cursor, Error> GetCursor() const;

	// Sets file offset to the current cursor position (in bytes)
	//    @param offset - offset in bytes from @origin
	//    @param origin - offset from the beginning of the stream
	//                    @offset will be added to this parameter
	//    @return - error code if failed
	virtual Optional<Error> MoveCursor(stream::Cursor offset,
	                                   stream::Position origin = stream::start);

	// Returns size of the stream buffer or error if failed,
	// returns "not_implemented" error if not overridend by child class
	virtual Result<size_t, Error> GetSize();

	// Operator '<<' applied to an output stream is known as insertion operator.
	// Use it for formatted human-readable text data output.
	OutputStream& operator << (OutputStream& stream);
	OutputStream& operator << (bool val);
	OutputStream& operator << (int16 val);
	OutputStream& operator << (uint16 val);
	OutputStream& operator << (int32 val);
	OutputStream& operator << (uint32 val);
	OutputStream& operator << (int64 val);
	OutputStream& operator << (uint64 val);
	OutputStream& operator << (float val);
	OutputStream& operator << (double val);
	OutputStream& operator << (long double val);
	OutputStream& operator << (void* val);
	OutputStream& operator << (const char* str);
	OutputStream& operator << (const String& str);

	// Virtual destructor.
	virtual ~OutputStream() = default;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//