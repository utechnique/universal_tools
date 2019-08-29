//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// class ut::Parameter<bool> is specialization for boolean types. This is
// a special case, because 'bool' type can have different size for different
// compilers.
template<>
class Parameter< bool > : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed object
	Parameter(bool* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return TypeName<bool>();
	}

	// Writes managed data to the stream
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(OutputStream& stream)
	{
		bool value = *static_cast<bool*>(ptr);
		byte b8value = value ? 1 : 0;
		return endian::Write<byte, skSerializationEndianness>(stream, &b8value);
	}

	// Loads managed data from the stream
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(InputStream& stream)
	{
		// read 1 byte
		byte b8value;
		Optional<Error> read_error = endian::Read<byte, skSerializationEndianness>(stream, &b8value);
		if (read_error)
		{
			return read_error;
		}

		// convert 1 byte to 'bool'
		bool* value_ptr = static_cast<bool*>(ptr);
		*value_ptr = b8value == 0 ? false : true;

		// success
		return Optional<Error>();
	}

	// Writes managed object data to the text node.
	//    @param node - text node to contain the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Tree<text::Node>& node)
	{
		// set node name
		SetTextNodeName(node);

		// value is a number
		node.data.value_type = String(TypeName<bool>());

		// print and save element
		const String text_form = Print<bool>(*(static_cast<bool*>(ptr)));
		node.data.value = text_form;

		// success
		return Optional<Error>();
	}

	// Loads managed object data from the text node.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(const Tree<text::Node>& node)
	{
		// scan element
		Result<String, Error> value_result = ExtractValueFromTextNode(node);
		if (value_result)
		{
			bool& element = *(static_cast<bool*>(ptr));
			element = Scan<bool>(value_result.GetResult());
		}
		else
		{
			return value_result.MoveAlt();
		}

		// success
		return Optional<Error>();
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//