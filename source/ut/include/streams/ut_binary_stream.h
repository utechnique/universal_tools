//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "streams/ut_input_stream.h"
#include "streams/ut_output_stream.h"
#include "containers/ut_array.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//

class BinaryStream : public InputStream, public OutputStream
{
public:
	// Constructor, cursor is set to ut::stream::Position::start
	BinaryStream();

	// Writes an array of @count elements, each one with a size of @size bytes,
	// from the block of memory pointed by @ptr to the current position
	//    @param ptr - pointer to the array of elements to be written
	//    @param size - size in bytes of each element to be written
	//    @param count - number of elements, each one with a size of @size bytes
	//    @return - ut::Error if encountered an error
	Optional<Error> Write(const void* ptr, size_t size, size_t count);

	// Reads an array of @count elements, each one with a size of @size bytes,
	// from the file and stores them in the block of memory specified by @ptr.
	//    @param ptr - pointer to a block of memory with a size of
	//                 at least (@size*@count) bytes
	//    @param size - Size, in bytes, of each element to be read
	//    @param count - Number of elements, each one with a size of @size bytes
	//    @return - ut::Error if encountered an error
	Optional<Error> Read(void* ptr, size_t size, size_t count);

	// Returns stream offset to the current cursor position (in bytes)
	//    @return - cursor position if file is opened, or error otherwise
	Result<stream::Cursor, Error> GetCursor() const;

	// Sets file offset to the current cursor position (in bytes)
	//    @param offset - offset in bytes from @origin
	//    @param origin - offset from the beginning of the stream
	//                    @offset will be added to this parameter
	//    @return - error code if failed
	Optional<Error> MoveCursor(stream::Cursor offset,
	                           stream::Position origin = stream::Position::start);

	// Returns size of the stream buffer or error if failed
	Result<size_t, Error> GetSize();

	// Returns a pointer to the stream buffer or error if failed
	Result<const void*, Error> GetData(stream::Cursor offset = 0) const;

	// Returns a constant reference to the array that contains stream buffer
	const Array<byte>& GetBuffer() const;

	// Makes a copy of the provided array and replaces current buffer with it
	void SetBuffer(const Array<byte>& copy);

	// Moves provided array and replaces current buffer with it
	void SetBuffer(Array<byte>&& rval);

protected:
	// current cursor position
	stream::Cursor cursor;

	// memory chunk
	Array<byte> data;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//