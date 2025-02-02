//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "error/ut_error_desc.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(error)
//----------------------------------------------------------------------------//
// Returns the description of the provided ut::error::Code value
//    @param error - error code
//    @return - text description of the @error code
String GetCodeDesc(Code error)
{
	String desc;
	desc.Print("Error # %u ", (uint32)error);
	switch(error)
	{
		case Code::fail:
			return desc + "(fail):\nCustom error, reason is unknown.";

		case Code::not_implemented:
			return desc + "(not implemented):\nFeature or interface is not "
				"implemented for current platform or configuration.";

		case Code::empty:
			return desc + "(empty):\nEntity is empty or uninitialized."
				"Implementation requires entity to be created/"
				"initialized. Contrary is considered an error.";

		case Code::out_of_memory:
			return desc + "(out of memory):\nNot enough memory is available "
				"for the attempted operation.";

		case Code::invalid_arg:
			return desc + "(invalid argument):\nFunction received invalid "
				"parameter value.";

		case Code::access_denied:
			return desc + "(access denied):\nPermission denied. Permission "
				"setting of the file (or any other entity) does not "
				"allow the specified access.";

		case Code::no_such_file:
			return desc + "(file not found):\nSpecified file or path not found.";

		case Code::not_supported:
			return desc + "(not supported):\nExternal device or feature is not "
				"supported by current platform or configuration.";

		case Code::protocol_error:
			return desc + "(protocol error):\nUnexpected behaviour or data "
				"during network or internal communication.";

		case Code::timed_out:
			return desc + "(timed out):\nThe time allocated for the operation "
				"has expired";

		case Code::already_exists:
			return desc + "(already exists):\nSpecified file or "
				"entity already exists.";

		case Code::busy:
			return desc + "(busy):\nSpecified file or entity is currently in use "
				"by the system/another process (or internal interface), "
				"and the implementation considers this an error.";

		case Code::not_empty:
			return desc + "(not empty):\nSpecified file or entity is not emty. "
				"This is considered an error, because implementation "
				"expectes only empty file or entity.";

		case Code::is_a_directory:
			return desc + "(is a directory):\nSpecified path is a directory, but "
				"implementation expects it to be a file.";

		case Code::not_a_directory:
			return desc + "(not a directory):\nSpecified path is not a directory, "
				"but implementation expects it to be a directory, not a file.";

		case Code::name_too_long:
			return desc + "(name is too long):\nSpecified name or path is too long.";

		case Code::address_in_use:
			return desc + "(address is in use):\nSpecific network address is already "
				"in use by another socket/entity.";

		case Code::not_found:
			return desc + "(not found):\nSpecified entity was not found.";

		case Code::abstract_class:
			return desc + "(abstract class):\nSpecified object can't be created or "
				"obtained because it's a pure abstract type.";

		case Code::out_of_bounds:
			return desc + "(out of bounds):\nSome kind of index value "
				"(such can be in array, map, stream, etc.) is out of range.";

		case Code::types_not_match:
			return desc + "(types don't match):\nSpecified entities have "
				"different types. Implementation requires both entities to "
				"have the same type.";

		case Code::connection_closed:
			return desc + "(connection closed):\nConnection was closed.";

		case Code::authorization:
			return desc + "(authorization failed):\nInvalid password or credentials.";

		default: return "(unknown error):\nSpecified error code is unknown.";
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(error)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//