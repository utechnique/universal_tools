//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "streams/ut_input_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Synchronizes the associated stream buffer with its controlled output sequence.
//    @return - error code if failed
Optional<Error> InputStream::Sync()
{
	return Error(error::not_supported);
}

// Returns stream offset to the current cursor position (in bytes)
//    @return - cursor position if file is opened, or error otherwise
Result<stream::Cursor, Error> InputStream::GetCursor() const
{
	return MakeError(error::not_supported);
}

// Sets file offset to the current cursor position (in bytes)
//    @param offset - offset in bytes from @origin
//    @param origin - offset from the beginning of the stream
//                    @offset will be added to this parameter
//    @return - error code if failed
Optional<Error> InputStream::MoveCursor(stream::Cursor offset,
                                        stream::Position origin)
{
	return Error(error::not_supported);
}

// Returns size of the stream buffer or error if failed,
// returns "not_implemented" error if not overridend by child class
Result<size_t, Error> InputStream::GetSize()
{
	return MakeError(error::not_implemented);
}

// Operator '>>' applied to an input stream is known as extraction operator.
// Use it for formatted human-readable text data input.
InputStream& InputStream::operator >> (InputStream& stream)
{
	return *this;
}

// Reads a text line into string from input buffer
// End of the line is either the end of the stream buffer or
// carriage return control character.
//    @return - line that was read from the stream, or error if failure occurred
Result<String, Error> InputStream::GetLine()
{
	// line to be read
	Array<char> line;

	// get platform specific cariage return character sequence
	const String carriage_ret = CarriageReturn<char>();
	const size_t cret_len = carriage_ret.Length();

	// indicates that at leas one character was read
	bool started = false;

	// iterate symbol by symbol
	while (true)
	{
		// symbol to be read
		char symbol;

		// read just one byte
		Optional<Error> read_error = Read(&symbol, 1, 1);

		// check the result
		if (read_error)
		{
			// if failed to read the first character - error occurred
			if (!started)
			{
				return MakeError(read_error.Move());
			}
			else
			{
				// otherwise it's ok, we have read the string up to the end of the stream
				// so it's time to break the loop to exit
				break;
			}
		}
		else
		{
			// if ok - then at leas one character has been read
			started = true;

			// add this character to the line
			line.Add(symbol);

			// check if our line ends with 'carriage return' character 
			bool ends_with_cret = true;
			size_t line_len = line.GetNum();
			for (uint32 i = 0; i < cret_len; i++)
			{
				if (line[line_len - cret_len + i] != carriage_ret[i])
				{
					ends_with_cret = false;
				}
			}

			// if it really end with 'carriage return' character - then
			// we have read the whole line, that's time to break the loop
			if (ends_with_cret)
			{
				for (uint32 i = 0; i < cret_len; i++)
				{
					line.Remove(line.GetNum() - 1);
				}
				break;
			}
		}
	}

	// add null terminator
	line.Add('\0');

	// convert to string
	return String(line.GetAddress());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//