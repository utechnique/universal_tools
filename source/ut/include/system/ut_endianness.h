//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "streams/ut_streams.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(endian)
//----------------------------------------------------------------------------//
// Endianness order.
enum order
{
	little = 0,
	big = 1
};

//----------------------------------------------------------------------------//
// Use this function to know endianness order of the current platform.
order GetNative();

//----------------------------------------------------------------------------//
// Use this function to read custom data in custom byte order from the stream.
//    @param endianness - byte order of the variable in memory.
//    @param stream - reference to the input stream.
//    @param data - address of the data to be read.
//    @param granularity - size of one element.
//    @param count - number of elements.
//    @return - ut::Error if failed.
template <order endianness>
Optional<Error> Read(InputStream& stream,
                     void* dst,
                     size_t granularity,
                     size_t count)
{
	// check if order is straight, note that this action is performed
	// only once during runtime for performance reason
	static const bool endianness_match = endianness == GetNative();

	// check if endianness is the same in stream buffer and memory, and if so - just read
	// bytes in forward order (making only one call to stream::Read function)
	if (endianness_match)
	{
		return stream.Read(dst, granularity, count);
	}
	else // otherwise bytes are read in reverse order one by one
	{
		byte* start = static_cast<byte*>(dst);
		for (size_t i = 0; i < count; i++)
		{
			for (size_t b = granularity; b--; )
			{
				Optional<Error> read_error = stream.Read(start + b, 1, 1);
				if (read_error)
				{
					return read_error;
				}
			}
			start += granularity;
		}
	}

	// success
	return Optional<Error>();
}

// Use this function to read stream data as a variable in custom byte order.
//    @param T - type of the variable to be read.
//    @param endianness - byte order of the variable in stream.
//    @param stream - reference to the input stream.
//    @param address - destination address of the variable to be read to.
//    @param count - number of elements.
//    @return - ut::Error if failed.
template <typename T, order endianness>
Optional<Error> Read(InputStream& stream, T* address, size_t count)
{
	return Read<endianness>(stream, address, sizeof(T), count);
}

//----------------------------------------------------------------------------//
// Use this function to write custom data in custom byte order to the stream.
//    @param T - type of the variable to be written.
//    @param endianness - byte order of the variable in memory.
//    @param stream - reference to the output stream.
//    @param data - address of the data to be written.
//    @param granularity - size of one element.
//    @param count - number of elements.
//    @return - ut::Error if failed.
template <order endianness>
Optional<Error> Write(OutputStream& stream,
                      const void* data,
                      size_t granularity,
                      size_t count)
{
	// check if order is straight, note that this action is performed
	// only once during runtime for performance reason
	static bool endianness_match = endianness == GetNative();

	// check if endianness is the same in stream buffer and memory, and if so - just write
	// bytes in forward order (making only one call to stream::Write function)
	if (endianness_match)
	{
		return stream.Write(data, 1, granularity * count);
	}
	else // otherwise bytes are written in reverse order one by one
	{
		const byte* start = static_cast<const byte*>(data);
		for (size_t i = 0; i < count; i++)
		{
			for (size_t b = granularity; b--; )
			{
				Optional<Error> write_error = stream.Write(start + b, 1, 1);
				if (write_error)
				{
					return write_error;
				}
			}
			start += granularity;
		}
	}

	// success
	return Optional<Error>();
}

// Use this function to write variables in custom byte order to the stream.
//    @param T - type of the variable to be written.
//    @param endianness - byte order of the variable in memory.
//    @param stream - reference to the output stream.
//    @param address - source address of the variable to be written.
//    @param count - number of elements.
//    @return - ut::Error if failed.
template <typename T, order endianness>
Optional<Error> Write(OutputStream& stream, const T* address, size_t count)
{
	return Write<endianness>(stream, address, sizeof(T), count);
}



//----------------------------------------------------------------------------//
END_NAMESPACE(endian)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//