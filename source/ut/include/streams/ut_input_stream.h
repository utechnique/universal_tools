//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
#include "streams/ut_base_stream.h"
#include <fstream>
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::InputStream is an abstract parent class for input streams.
class InputStream : public BaseStream
{
public:
	// Reads an array of @count elements, each one with a size of @size bytes,
	// from the file and stores them in the block of memory specified by @ptr.
	//    @param ptr - pointer to a block of memory with a size of
	//                 at least (@size*@count) bytes
	//    @param size - Size, in bytes, of each element to be read
	//    @param count - Number of elements, each one with a size of @size bytes
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Read(void* ptr, size_t size, size_t count) = 0;

	// Synchronizes the associated stream buffer with its controlled input sequence.
	//    @return - error code if failed
	virtual Optional<Error> Sync();

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

	// Operator '>>' applied to an input stream is known as extraction operator.
	// Use it for formatted human-readable text data input.
	InputStream& operator >> (InputStream& stream);

	// Reads a text line into string from input buffer
	// End of the line is either the end of the stream buffer or
	// 'carriage return'(\n or \r\n) control character.
	//    @return - line that was read from the stream, or error if failure occurred
	Result<String, Error> GetLine();

	// Virtual destructor.
	virtual ~InputStream() = default;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//