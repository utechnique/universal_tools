//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::stream is a namespace for stream types and enumerations.
namespace stream
{
	// ut::stream::Cursor is integer type
	// to navigate through the ut::Stream class.
	typedef size_t Cursor;

	// ut::stream::Position is enumaration of position labels for
	// the ut::stream::Cursor while navigating inside a stream.
	enum Position
	{
		cursor = 0,
		start = 1,
		end = 2,
	};
}

//----------------------------------------------------------------------------//
// ut::BaseStream is an abstract parent class for all i/o streams.
// Stream could be a file on a hard disc, or a chunk of bytes in RAM, or any
// other entity suitable for input/output actions.
class BaseStream
{
public:
	// Returns stream offset to the current cursor position (in bytes)
	//    @return - cursor position if file is opened, or error otherwise
	virtual Result<stream::Cursor, Error> GetCursor() const = 0;

	// Sets file offset to the current cursor position (in bytes)
	//    @param offset - offset in bytes from @origin
	//    @param origin - offset from the beginning of the stream
	//                    @offset will be added to this parameter
	//    @return - error code if failed
	virtual Optional<Error> MoveCursor(stream::Cursor offset,
	                                   stream::Position origin = stream::start) = 0;

	// Returns size of the stream buffer or error if failed
	virtual Result<size_t, Error> GetSize() = 0;

	// ut::Stream is a polymorphic type
	virtual ~BaseStream() {};
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//