//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_ref.h"
#include "containers/ut_pair.h"
#include "text/ut_document.h"
#include "meta/ut_meta_info.h"
#include "meta/ut_meta_node.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Forward declarations.
class Snapshot;

//----------------------------------------------------------------------------//
// ut::meta::Controller is a class that helps to serialize/deserialize data.
// It unifies text and binary forms of representation - the same member function
// can be used to write/read data (WriteValue() or ReadValue() for example) both
// for text nodes and binary streams. Note that controller can work only in one
// mode at the same time. Call SetBinaryInputStream() or SetBinaryOutputStream()
// or SetTextInputNode() or SetTextOutputNode() to set correct mode before using
class Controller
{
public:
	// Enumeration of possible modes, all modes are mutually exclusive
	enum Mode
	{
		empty_mode = 0,
		binary_input_mode = 1,
		binary_output_mode = 2,
		text_input_mode = 3,
		text_output_mode = 4
	};

	// Union of pointers to possible input/output
	// source/target for serialization/deserialization.
	union IO
	{
		InputStream* binary_input;
		OutputStream* binary_output;
		const Tree<text::Node>* text_input;
		Tree<text::Node>* text_output;
	};

	// type for size information (number of parameters, parameter size, etc.)
	typedef uint32 SizeType;

	// Constructor
	//    @param info_copy - copy of the serialization info, that will be
	//                       used during serialization and deserialization
	Controller(const Info& info_copy = Info::CreateComplete());

	// Copy constructor
	Controller(const Controller& copy);

	// Assignment operator
	Controller& operator = (const Controller& copy);

	// Extracts a custom entity value from the node.
	// Note that this information may be absent.
	//    @param node_name - name of the child node containing desired value
	//    @return - value of desired node, or ut::Error if encountered an error
	Result<String, Error> ExtractTextNodeValue(const Tree<text::Node>& parent_node,
	                                           const String& node_name) const;

	// Changes mode to binary input stream
	//    @param stream - reference to input stream to read data from
	void SetBinaryInputStream(InputStream& stream);

	// Changes mode to binary output stream
	//    @param stream - reference to output stream to write data to
	void SetBinaryOutputStream(OutputStream& stream);

	// Changes mode to text input node
	//    @param node - reference to the text node to read data from
	void SetTextInputNode(const Tree<text::Node>& node);

	// Changes mode to text output node
	//    @param node - reference to the text node to write data to
	void SetTextOutputNode(Tree<text::Node>& node);

	// Returns current mode
	Mode GetMode() const;

	// Returns serialization information
	Info GetInfo() const;

	// Serializes a provided reflective node.
	//    @param node - a reference to the ut::meta::Snapshot object to be
	//                  serialized, it can be created by calling
	//                  ut::meta::Snapshot::Capture() function.
	//    @param initialize - this boolean indicates if node will be initialized
	//                        with special 'header' information: serialization
	//                        info, shared objects, etc.
	//    @return - ut::Error if failed.
	Optional<Error> WriteNode(Snapshot& node, bool initialize = true);

	// Deserializes a provided reflective node.
	//    @param node - a reference to the ut::meta::Snapshot object to be
	//                  deserialized, it can be created by calling
	//                  ut::meta::Snapshot::Capture() function.
	//    @param initialize - this boolean indicates if node will be initialized
	//                        with special 'header' information: serialization
	//                        info, shared objects, etc.
	//    @return - ut::Error if failed.
	Optional<Error> ReadNode(Snapshot& node, bool initialize = true);

	// Writes attribute, "attribute" here means something that is not representing
	// value directly, but helps to create this value
	//    @param element - reference to the value to be written
	//    @param attribute_name - name of the attribute
	//    @param is_attribute - optional bool value indicating whether
	//                          text node will be marked as "attribute"
	//    @return - ut::Error if failed
	template <typename T>
	Optional<Error> WriteAttribute(const T& element,
	                               const String& attribute_name,
	                               bool is_attribute = true)
	{
		// write binary form
		if (mode == binary_output_mode)
		{
			Optional<Error> write_error = WriteBinary<T>(&element, 1);
			if (write_error)
			{
				return write_error;
			}
		}
		else if (mode == text_output_mode) // write text form
		{
			// create new text node or use existing one
			Optional< Ref< Tree<text::Node> > > find_result = FindTextNode<Ref>(*io.text_output, attribute_name);
			if (find_result)
			{
				// use existing node to overwrite value
				Ref< Tree<text::Node> > text_node = find_result.Get();
				text_node->data.value = Print<T>(element);
			}
			else
			{
				// create a new node
				Tree<text::Node> attribute_node;
				attribute_node.data.name = attribute_name;
				attribute_node.data.value = Print<T>(element);
				attribute_node.data.is_attribute = is_attribute;

				// set value type
				attribute_node.data.value_type = String(TypeName<T>());

				// add text node to the tree
				if (!io.text_output->Add(Move(attribute_node)))
				{
					return Error(error::out_of_memory);
				}
			}
		}

		// success
		return Optional<Error>();
	}

	// Reads attribute, "attribute" here means something that is not representing
	// value of the parameter directly, but helps to create this value
	//    @param attribute_name - name of the attribute
	//    @return - attribute value or ut::Error if failed
	template <typename T>
	Result<T, Error> ReadAttribute(const String& attribute_name)
	{
		// create a new instance of element
		T element;

		// read binary form
		if (mode == binary_input_mode)
		{
			Optional<Error> read_error = ReadBinary<T>(&element, 1);
			if (read_error)
			{
				return MakeError(read_error.Move());
			}
		}
		else if (mode == text_input_mode) // read text form
		{
			Result<String, Error> extraction_result = ExtractTextNodeValue(*io.text_input, attribute_name);
			if (!extraction_result)
			{
				return MakeError(extraction_result.MoveAlt());
			}

			element = Scan<T>(extraction_result.GetResult());
		}
		else
		{
			return MakeError(Error(error::fail, "Invalid (non-input) mode."));
		}

		// success
		return element;
	}

	// Writes value of the parameter
	//    @param element - reference to the value to be written
	//    @return - ut::Error if failed
	template <typename T>
	Optional<Error> WriteValue(const T& element)
	{
		if (mode == text_output_mode && !info.HasValueEncapsulation())
		{
			io.text_output->data.value = Print<T>(element);
			io.text_output->data.value_type = String(TypeName<T>());
			io.text_output->data.encapsulation_name = String(node_names::skValue);
			return Optional<Error>();
		}
		else
		{
			return WriteAttribute<T>(element, node_names::skValue, false);
		}
	}

	// Reads value of the parameter
	//    @return - value or ut::Error if failed
	template <typename T>
	Result<T, Error> ReadValue()
	{
		if (mode == text_input_mode && !info.HasValueEncapsulation())
		{
			Result<String, Error> extraction_result = ExtractTextNodeValue(*io.text_input, node_names::skValue);
			T element = Scan<T>(extraction_result ? extraction_result.GetResult() : io.text_output->data.value.Get());
			return element;
		}
		else
		{
			return ReadAttribute<T>(node_names::skValue);
		}
	}

	// Searches for a child text node by name
	//    @param parent_node - reference to the parent text node to search in
	//    @param node_name - name of the node to search for
	//    @return - reference to the text node, or ut::Error if failed to find
	template<template<class> class RefContainer>
	Optional< RefContainer< Tree<text::Node> > > FindTextNode(RefContainer< Tree<text::Node> > parent_node,
	                                                              const String& node_name) const
	{
		// search for a desired node
		for (size_t i = 0; i < parent_node->GetNumChildren(); i++)
		{
			// skip if name doesn't match
			if (node_name != parent_node.Get()[i].data.name)
			{
				continue;
			}

			// success
			return RefContainer< Tree<text::Node> >(parent_node.Get()[i]);
		}

		// error - not found a node with such a name
		return Optional< RefContainer< Tree<text::Node> > >();
	}

private:
	// Writes ut::meta::Info data, shared objects and linkage data.
	//    @param node - reference to the node to initialize.
	//    @return - ut::Error if failed.
	Optional<Error> WriteInitializationData(Snapshot& node);

	// Reads ut::meta::Info and linkage data, creates shared objects.
	//    @param node - reference to the node to initialize.
	//    @return - ut::Error if failed.
	Optional<Error> ReadInitializationData(Snapshot& node);

	// Writes attributes that are mandatory for all parameters,
	// like name, type, id, etc.
	//    @param node - reference to the node that is being serialized.
	//    @return - ut::Error if failed.
	Optional<Error> WriteUniformAttributes(Snapshot& node);

	// Reads attributes that are mandatory for all parameters,
	// like name, type, id, etc.
	//    @param node - reference to the node that is being deserialized.
	//    @param out_node_name -  reference to the ut::Optional container where
	//                            a name of the node will be read to.
	//    @param out_type_name -  reference to the ut::Optional container where
	//                            a type of the node will be read to.
	//    @param out_id -  reference to the ut::Optional container where
	//                     an id of the node will be read to.
	//    @param node - reference to the node that is being deserialized.
	//    @return - ut::Error if failed.
	Optional<Error> ReadUniformAttributes(Snapshot& node,
	                                      Optional<String>& out_node_name,
	                                      Optional<String>& out_type_name,
	                                      Optional<SizeType>& out_id);

	// Writes parameter and all child nodes of this parameter.
	//    @param node - reference to the node that is being serialized.
	//    @return - ut::Error if failed.
	Optional<Error> WriteParameter(Snapshot& node);

	// Reads parameter and all child nodes of this parameter.
	//    @param node - reference to the node that is being deserialized.
	//    @return - ut::Error if failed.
	Optional<Error> ReadParameter(Snapshot& node);

	// Serializes leaves of the provided reflective node.
	//    @param node - reference to a parent node.
	//    @return - ut::Error if failed.
	Optional<Error> WriteChildNodes(Snapshot& node);

	// Deserializes leaves of the provided reflective node.
	//    @param node - reference to a parent node.
	//    @return - ut::Error if failed.
	Optional<Error> ReadChildNodes(Snapshot& node);

	// Writes a name of the node.
	//    @param name - name of the node.
	//    @return - ut::Error if failed.
	Optional<Error> WriteNodeName(const String& name);

	// Reads a name of the node.
	//    @param name - name of the node.
	//    @return ut::Error if failed.
	Result<Optional<String>, Error> ReadNodeName();

	// Writes a number of leaves in a node.
	//    @param count - number of leaves to be written.
	//    @return - ut::Error if failed.
	Optional<Error> WriteNumberOfChildNodes(size_t count);

	// Reads a serialized number of leaves in a node.
	//    @param count - number of leaves to be read.
	//    @return - ut::Error if failed.
	Result<size_t, Error> ReadNumberOfChildNodes(const Snapshot& node);

	// Allocates space in the output binary stream for the size variable.
	//    @return - position of the size variable in a stream, or
	//              ut::Error if something failed.
	Result<stream::Cursor, Error> ReserveParameterSize();

	// Writes a size of a parameter to the binary stream.
	//    @param start_position - position of the preallocated space for
	//                            the size variable in a stream.
	//    @return - ut::Error if failed.
	Optional<Error> WriteParameterSize(stream::Cursor start_position);

	// Reads a serialized size of a parameter from the binary stream.
	//    @return - cursor position of the next parameter or ut::Error if failed.
	Result<stream::Cursor, Error> ReadParameterSize();

	// Checks if name of the provided node matches provided name, and if not - searches
	// for a sibling node with such a name.
	//    @param node - reference to a node to check.
	//    @param name - desired node name.
	//    @return - a reference to the desired node or ut::Error if failed.
	Result<Ref<Snapshot>, Error> FindSiblingNode(Snapshot& node, const Optional<String>& name);

	// Checks if provided type names match.
	//    @parameter node - reference to the node that contains parameter type.
	//    @parameter serialized_type - name of a parameter that was deserialized
	//                                 from a stream or a text node.
	//    @return - ut::Error if failed.
	Optional<Error> CheckType(const Snapshot& node, const String& serialized_type);

	// Moves an input stream cursor to the next parameter.
	//    @param next - stream position of the next parameter.
	//    @return - ut::Error if failed.
	Optional<Error> SkipParameter(stream::Cursor next);

	// Returns 'true' if parameters can be skipped without error if something went wrong.
	bool SkipIsPossible() const;

	// Searches for a desired node inside current text node, and changes input/output
	// source/target to this value node.
	//    @param name - name of the node.
	//    @return - ut::Error if failed.
	Optional<Error> DiveIntoNamedNode(const String& name);

	// Creates (or loads a child node) a new text node, and changes
	// input/output source/target to this node.
	//    @parameter id - id of the child node to load, this id
	//                    is needed only for the input text variant
	//    @return - ut::Error if failed.
	Optional<Error> DiveIntoNewNode(size_t id);

	// Writes serialization information about the current serialization node.
	// Information is written to the special attribute node (it's name is 
	// defined here: ut::meta::node_names::skInfo).
	//     @return - ut::Error if failed.
	Optional<Error> WriteInfo();

	// Reads serialization information about the current serialization node
	// and applies correct settings to the controller.
	//    @return - ut::Error if failed.
	Optional<Error> ReadInfo();

	// Returns a current state.
	Controller SaveState();

	// Loads a provided state.
	void LoadState(const Controller& controller);

	// Helper function to read custom value from the binary stream
	//    @address - pointer to the element to be read
	//    @count - number of elements to read
	//    @return - ut::Error if failed
	template <typename T>
	Optional<Error> ReadBinary(T* address, size_t count)
	{
		if (info.GetEndianness() == endian::little)
		{
			return endian::Read<T, endian::little>(*io.binary_input, address, count);
		}
		else
		{
			return endian::Read<T, endian::big>(*io.binary_input, address, count);
		}
	}

	// Helper function to write custom value to the binary stream
	//    @address - pointer to the element to be written
	//    @count - number of elements to write
	//    @return - ut::Error if failed
	template <typename T>
	Optional<Error> WriteBinary(const T* address, size_t count)
	{
		if (info.GetEndianness() == endian::little)
		{
			return endian::Write<T, endian::little>(*io.binary_output, address, count);
		}
		else
		{
			return endian::Write<T, endian::big>(*io.binary_output, address, count);
		}
	}

	// Overloaded function to read ut::String from the binary stream.
	// Every character is read separately to preserve endianness.
	//    @str_ptr - pointer to string to be read
	//    @count - number of strings to be read
	//    @return - ut::Error if failed
	template <typename T> Optional<Error> ReadBinary(String* str_ptr, size_t count)
	{
		for (size_t str_id = 0; str_id < count; str_id++)
		{
			// get string reference from the pointer
			String& str = str_ptr[str_id];

			// clear the string
			str.Empty();

			// read symbol by symbol and exit after
			// meeting a null-terminator
			char c;
			do
			{
				Optional<Error> read_error = ReadBinary<char>(&c, 1);
				if (read_error)
				{
					return read_error;
				}

				str.Add(c);
			} while (c != '\0');
		}

		// success
		return Optional<Error>();
	}

	// Overloaded function to write ut::String to the binary stream.
	// Every character is written separately to preserve endianness.
	//    @address - pointer to the string to be written
	//    @count - number of strings to write
	//    @return - ut::Error if failed
	template <typename T> Optional<Error> WriteBinary(const String* str_ptr, size_t count)
	{
		for (size_t str_id = 0; str_id < count; str_id++)
		{
			// get string object and calculate it's length
			const String& str = str_ptr[str_id];
			size_t len = str.Length() + 1;

			// write a string character by character
			const char* start = str.GetAddress();
			for (size_t char_id = 0; char_id < len; char_id++)
			{
				Optional<Error> write_error = WriteBinary<char>(start + char_id, 1);
				if (write_error)
				{
					return write_error;
				}
			}
		}

		// success
		return Optional<Error>();
	}

	// Overloaded function to read boolean from the binary stream.
	// Size of the 'bool' type is compiler-specific, so it's read/written
	// with a conversion to the 'ut::byte' type.
	//    @ptr - pointer to boolean value to be read
	//    @count - number of values to be read
	//    @return - ut::Error if failed
	template <typename T> Optional<Error> ReadBinary(bool* ptr, size_t count)
	{
		// read 1 byte
		byte b8value;
		Optional<Error> read_error = ReadBinary<byte>(&b8value, 1);
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

	// Overloaded function to read boolean from the binary stream.
	// Size of the 'bool' type is compiler-specific, so it's read/written
	// with a conversion to the 'ut::byte' type.
	//    @ptr - pointer to the boolean value to be written
	//    @count - number of values to write
	//    @return - ut::Error if failed
	template <typename T> Optional<Error> WriteBinary(const bool* ptr, size_t count)
	{
		// convert bool variable to one byte
		bool value = *static_cast<const bool*>(ptr);
		byte b8value = value ? 1 : 0;
		return WriteBinary<byte>(&b8value, 1);
	}

	// Serialization info contains information how to 
	// serialize/deserialize data.
	Info info;

	// Current mode.
	Mode mode;

	// Union of pointers to possible input/output
	// entities for serializtion/deserialization.
	IO io;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//