//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "streams/ut_binary_stream.h"
#include "system/ut_memory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Constructor, cursor is set to ut::stream::start
BinaryStream::BinaryStream() : cursor(0)
{ }

//----------------------------------------------------------------------------->
// Writes an array of @count elements, each one with a size of @size bytes,
// from the block of memory pointed by @ptr to the current position
//    @param ptr - pointer to the array of elements to be written
//    @param size - size in bytes of each element to be written
//    @param count - number of elements, each one with a size of @size bytes
//    @return - ut::Error if encountered an error
Optional<Error> BinaryStream::Write(const void* ptr,
                                    size_t size,
                                    size_t count)
{
	// calculate full array size
	const size_t arr_size = size * count;

	// allocate enough memory
	const size_t min_size = cursor + arr_size;
	if (data.GetSize() < min_size)
	{
		if (!data.Resize(min_size))
		{
			return Error(error::out_of_memory);
		}
	}

	// write data
	memory::Copy(data.GetAddress() + cursor, ptr, arr_size);

	// move cursor
	cursor += arr_size;

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads an array of @count elements, each one with a size of @size bytes,
// from the file and stores them in the block of memory specified by @ptr.
//    @param ptr - pointer to a block of memory with a size of
//                 at least (@size*@count) bytes
//    @param size - Size, in bytes, of each element to be read
//    @param count - Number of elements, each one with a size of @size bytes
//    @return - ut::Error if encountered an error
Optional<Error> BinaryStream::Read(void* ptr, size_t size, size_t count)
{
	// calculate full array size
	const size_t arr_size = size * count;

	// check boundaries
	const size_t min_size = cursor + arr_size;
	if (data.GetSize() < min_size)
	{
		return Error(error::out_of_bounds);
	}

	// read data
	memory::Copy(ptr, data.GetAddress() + cursor, arr_size);

	// move cursor
	cursor += arr_size;

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns stream offset to the current cursor position (in bytes)
//    @return - cursor position if file is opened, or error otherwise
Result<stream::Cursor, Error> BinaryStream::GetCursor() const
{
	return cursor;
}

//----------------------------------------------------------------------------->
// Sets file offset to the current cursor position (in bytes)
//    @param offset - offset in bytes from @origin
//    @param origin - offset from the beginning of the stream
//                    @offset will be added to this parameter
//    @return - error code if failed
Optional<Error> BinaryStream::MoveCursor(stream::Cursor offset, stream::Position origin)
{
	// calculate start position
	stream::Cursor start;
	switch(origin)
	{
		case stream::cursor: start = cursor; break;
		case stream::start: start = 0; break;
		case stream::end: start = data.GetSize(); break;
		default: return Error(error::invalid_arg);
	}

	// validate new cursor value
	stream::Cursor new_cursor = start + offset;
	if (new_cursor > data.GetSize())
	{
		return Error(error::invalid_arg);
	}

	// set new cursor value
	cursor = new_cursor;

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns size of the stream buffer or error if failed
Result<size_t, Error> BinaryStream::GetSize()
{
	return data.GetSize();
}

//----------------------------------------------------------------------------->
// Returns the pointer to the stream buffer or error if failed
Result<const void*, Error> BinaryStream::GetData(stream::Cursor offset) const
{
	if (data.Count() != 0)
	{
		return data.GetAddress() + offset;
	}
	else
	{
		return MakeError(error::empty);
	}
}

//----------------------------------------------------------------------------->
// Returns a constant reference to the array that contains stream buffer
const Array<byte>& BinaryStream::GetBuffer() const
{
	return data;
}

//----------------------------------------------------------------------------->
// Makes a copy of the provided array and replaces current buffer with it
void BinaryStream::SetBuffer(const Array<byte>& copy)
{
	data = copy;
}


//----------------------------------------------------------------------------->
// Moves provided array and replaces current buffer with it
void BinaryStream::SetBuffer(Array<byte>&& rval)
{
	data = rval;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//