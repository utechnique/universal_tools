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
#include "meta/linkage/ut_meta_link_cache.h"
#include "pointers/ut_shared_ptr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Forward declarations.
class Snapshot;
class Linker;

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
	// type for size information (number of parameters, parameter size, etc.)
	typedef uint32 SizeType;

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

	// Contains attributes that are mandatory for all parameters.
	struct Uniform
	{
		Optional<String> name;
		Optional<String> type;
		Optional<SizeType> id;
		Optional<stream::Cursor> next;
	};

	// Represents a combination of possible ways how a
	// node can be read/written.
	struct SerializationOptions
	{
		// specifies that a node  has special 'header' information: 
		// serialization info, shared objects, etc; if this parameter 
		// is 'true' - then @only_uniforms parameter must be 'false'
		bool initialize = true;

		// specifies that node's body must be skipped
		// and only uniform data is to be read/written
		bool only_uniforms = false;

		// specifies that a node must have size information even
		// if info.HasSizeinformations() returns 'false' 
		bool force_size_info = false;
	};

	// Constructor
	//    @param info_copy - copy of the serialization info, that will be
	//                       used during serialization and deserialization
	Controller(const Info& info_copy = Info::CreateComplete());

	// Extracts a custom entity value from the node.
	// Note that this information may be absent.
	//    @param node_name - name of the child node containing desired value
	//    @return - value of desired node, or nothing if encountered an error
	Optional<String> ExtractTextNodeValue(const Tree<text::Node>& parent_node,
	                                      const String& node_name) const;

	// Changes mode to binary input stream
	//    @param stream - reference to input stream to read data from
	Optional<Error> SetBinaryInputStream(InputStream& stream);

	// Changes mode to binary output stream
	//    @param stream - reference to output stream to write data to
	Optional<Error> SetBinaryOutputStream(OutputStream& stream);

	// Changes mode to text input node
	//    @param node - reference to the text node to read data from
	Optional<Error> SetTextInputNode(const Tree<text::Node>& node);

	// Changes mode to text output node
	//    @param node - reference to the text node to write data to
	Optional<Error> SetTextOutputNode(Tree<text::Node>& node);

	// Returns current mode
	Mode GetMode() const;

	// Returns serialization information
	Info GetInfo() const;

	// Returns @cursor value.
	stream::Cursor GetCursor() const;

	// Returns current cursor position of the input/output binary stream.
	// If controller is in text mode - returns @cursor value.
	// Note that this value can differ from Controller::GetCursor() function result.
	Result<stream::Cursor, Error> GetStreamCursor();

	// Assigns a provided value to the @cursor member variable.
	//    @param position - value of the cursor to be set.
	//    @param sync - boolean whether to sync binary stream afterwards or not.
	//    @return - error if failed.
	Optional<Error> SetCursor(stream::Cursor position, bool sync = false);

	// Synchronizes binary stream with @cursor position.
	// Does nothing if in text mode.
	//    @return - error if failed.
	Optional<Error> Sync();

	// Synchronizes @cursor position with binary stream.
	// Does nothing if in text mode.
	//    @return - error if failed.
	Optional<Error> SyncWithStream();

	// Creates a task for linker to write a correct id of the linked
	// object (that is defined as a pointer) into the value node.
	//    @param parameter - pointer to the parameter representing a link,
	//                       (raw pointer, shared/weak ptr, etc.).
	//    @param linked_address - adress of the linked object.
	//    @return - ut::Error if failed.
	Optional<Error> WriteLink(const BaseParameter* parameter,
	                          const void* linked_address);

	// Creates a task for linker to read an id of the linked
	// object (that is defined as a pointer) from the value node,
	// and to link it with the provided parameter.
	//    @param parameter - pointer to the parameter representing a link,
	//                       (raw pointer, shared/weak ptr, etc.).
	//    @return - ut::Error if failed.
	Optional<Error> ReadLink(const BaseParameter* parameter);

	// Creates a task for linker to read an id of the linked
	// shared object from the value node, and to link it with
	// the provided parameter.
	//    @param parameter - pointer to the parameter representing a link,
	//                       (shared ptr).
	//    @param ptr - shared pointer to the holder of the SharedPtr object.
	//    @return - ut::Error if failed.
	Optional<Error> ReadSharedLink(const BaseParameter* parameter,
	                               const SharedPtr<class SharedPtrHolderBase>& ptr);

	// Creates a task for linker to read an id of the linked
	// shared object from the value node, and to link it with
	// the provided parameter.
	//    @param parameter - pointer to the parameter representing a link,
	//                       (weak ptr).
	//    @return - ut::Error if failed.
	Optional<Error> ReadWeakLink(const BaseParameter* parameter);

	// Serializes a provided reflective node.
	//    @param node - a reference to the ut::meta::Snapshot object to be
	//                  serialized, it can be created by calling
	//                  ut::meta::Snapshot::Capture() function.
	//    @param options - options specifying how exactly @node is supposed to
	//                     be written.
	//    @return - ut::Error if failed.
	Optional<Error> WriteNode(Snapshot& node, const SerializationOptions& options);

	// Deserializes a provided reflective node.
	//    @param node - a reference to the ut::meta::Snapshot object to be
	//                  deserialized, it can be created by calling
	//                  ut::meta::Snapshot::Capture() function;
	//                  this parameter can be empty if @skip_loading is 'true'.
	//    @param options - options specifying how exactly @node is supposed to
	//                     be read.
	//    @return - ut::meta::Controller::Uniform object describing a node
	//              or ut::Error if failed.
	Result<Uniform, Error> ReadNode(Optional<Snapshot&> node, const SerializationOptions& options);

	// Adds unique shared parameter.
	//    @param ptr - shared pointer to the holder of the SharedPtr object.
	//    @param address - address of the shared object.
	//    @return - ut::Error if failed.
	Optional<Error> WriteSharedObject(const SharedPtr<class SharedPtrHolderBase>& ptr,
	                                  const void* address);

	// Reads binary data from the current value node.
	//    @param dst - pointer to the destination binary data.
	//    @param size - size of the data in bytes.
	//    @param granularity - size of one element in the provided data.
	//    @return - ut::Error if failed.
	Optional<Error> ReadBinaryValue(void* dst,
	                                SizeType size,
	                                SizeType granularity);

	// Writes provided binary data to the current value node.
	//    @param data - pointer to the binary data to be written.
	//    @param size - size of the data in bytes.
	//    @param granularity - size of one element in the provided data.
	//    @return - ut::Error if failed.
	Optional<Error> WriteBinaryValue(const void* data,
	                                 SizeType size,
	                                 SizeType granularity);	

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
				attribute_node.data.value_type = String(Type<T>::Name());

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
			Optional<String> extraction_result = ExtractTextNodeValue(*io.text_input, attribute_name);
			if (!extraction_result)
			{
				return MakeError(error::not_found);
			}

			element = Scan<T>(extraction_result.Get());
		}
		else
		{
			return MakeError(error::fail, "Invalid (non-input) mode.");
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
			io.text_output->data.value_type = String(Type<T>::Name());
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
			// try to find a value in a separate "value" node (variant for json)
			Optional<String> extraction_result = ExtractTextNodeValue(*io.text_input, node_names::skValue);
			
			// extract element from string
			T element;
			if (extraction_result)// we have found "value" node - totally ok!
			{
				element = Scan<T>(extraction_result.Get());
			}
			else if(io.text_output->data.value) // there is no separate "value" node, it can be ok,
			{                                   // if it'a a json document e.g.
				element = Scan<T>(io.text_output->data.value.Get());
			}
			else // no "value" node and no value inside a current node..
			{    // the only thing we can do - to parse an empty string
				element = Scan<T>(String());
			}

			// final value
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
		for (size_t i = 0; i < parent_node->CountChildren(); i++)
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
	// Initializes intermediate modules (such as linker) before reading/writing a node.
	//    @param node - reference to the node to initialize.
	//    @return - ut::Error if failed.
	Optional<Error> InitializeNode(Snapshot& node);

	// Finalizes intermediate modules (such as linker) after a node has been read/written.
	//    @param node - reference to the node to finalize.
	//    @return - ut::Error if failed.
	Optional<Error> FinalizeNode(Snapshot& node);

	// Writes ut::meta::Info data, shared objects and linkage data.
	//    @param node - reference to the node to initialize.
	//    @return - ut::Error if failed.
	Optional<Error> WriteInitializationData(Snapshot& node);

	// Reads ut::meta::Info and linkage data, creates shared objects.
	//    @param node - reference to the node to initialize.
	//    @return - ut::Error if failed.
	Optional<Error> ReadInitializationData(Snapshot& node);

	// Writes attributes that are mandatory for all parameters,
	// like name, type, id, size etc.
	//    @param node - reference to the node that is being serialized.
	//    @return - Optional stream offset to the size variable
	//              or ut::Error if failed.
	Result<Optional<stream::Cursor>, Error> WriteUniformAttributes(Snapshot& node);

	// Reads attributes that are mandatory for all parameters,
	// like name, type, id, etc.
	//    @return - meta::Controller::Uniform object ot ut::Error if failed.
	Result<Uniform, Error> ReadUniformAttributes();

	// Writes parameter and all child nodes of this parameter.
	//    @param node - reference to the node that is being serialized.
	//    @param start - start position of the node in the binary stream.
	//    @return - ut::Error if failed.
	Optional<Error> WriteParameter(Snapshot& node, stream::Cursor start);

	// Reads parameter and all child nodes of this parameter.
	//    @param node - reference to the node that is being deserialized.
	//    @param start - start position of the node in the binary stream.
	//    @return - ut::Error if failed.
	Optional<Error> ReadParameter(Snapshot& node, stream::Cursor start);

	// Serializes leaves of the provided reflective node.
	//    @param node - reference to a parent node.
	//    @param start - start position of the node in the binary stream.
	//    @return - ut::Error if failed.
	Optional<Error> WriteChildNodes(Snapshot& node, stream::Cursor start);

	// Deserializes leaves of the provided reflective node.
	//    @param node - reference to a parent node.
	//    @param start - start position of the node in the binary stream.
	//    @return - ut::Error if failed.
	Optional<Error> ReadChildNodes(Snapshot& node, stream::Cursor start);

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

	// Allocates space for text child nodes (leaves) so that
	// every child node had stable address.
	//    @param count - number of child nodes.
	//    @return - id of the first child, or error if something went wrong.
	Result<size_t, Error> AllocateChildNodes(size_t count);

	// Allocates space in the output binary stream for the size variable.
	//    @return - position of the size variable in a stream, or
	//              ut::Error if something failed.
	Result<Optional<stream::Cursor>, Error> ReserveParameterSize();

	// Writes a size of a parameter to the binary stream.
	//    @param start_position - position of the preallocated space for
	//                            the size variable in a stream.
	//    @return - ut::Error if failed.
	Optional<Error> WriteParameterSize(const Optional<stream::Cursor>& start_position);

	// Reads a serialized size of a parameter from the binary stream.
	//    @return - cursor position of the next parameter or ut::Error if failed.
	Result<stream::Cursor, Error> ReadParameterSize();

	// Checks if name of the provided node matches provided name, and if not - searches
	// for a sibling node with such a name.
	//    @param node - reference to a node to check.
	//    @param name - desired node name.
	//    @return - a reference to the desired node or ut::Error if failed.
	Optional<Snapshot&> FindSiblingNode(Snapshot& node, const Optional<String>& name);

	// Checks if provided type names match.
	//    @parameter node - reference to the node that contains parameter type.
	//    @parameter serialized_type - name of a parameter that was deserialized
	//                                 from a stream or a text node.
	//    @return - ut::Error if failed.
	Optional<Error> CheckType(const Snapshot& node, const String& serialized_type);

	// Moves an input stream cursor to the next parameter.
	//    @param next - stream position of the next parameter.
	//    @return - ut::Error if failed.
	Optional<Error> SkipParameter(const Optional<stream::Cursor>& next);

	// Returns 'true' if parameters can be skipped without error if something went wrong.
	bool SkipIsPossible() const;

	// Searches for a desired node inside current text node, and changes input/output
	// source/target to this value node.
	//    @param name - name of the node.
	//    @return - ut::Error if failed.
	Optional<Error> DiveIntoNamedNode(const String& name);

	// Dives into a text node that is defined by an id, and changes
	// input/output source/target to this node.
	//    @parameter id - id of the child node to dive in.
	//    @return - ut::Error if failed.
	Optional<Error> DiveIntoChildNode(size_t id);

	// Writes serialization information about the current serialization node.
	// Information is written to the special attribute node (it's name is 
	// defined here: ut::meta::node_names::skInfo).
	//     @return - ut::Error if failed.
	Optional<Error> WriteInfo();

	// Reads serialization information about the current serialization node
	// and applies correct settings to the controller.
	//    @return - ut::Error if failed.
	Optional<Error> ReadInfo();

	// Writes shared objects, these objects have no owner,
	// so must be written separately.
	//    @return - ut::Error if failed.
	Optional<Error> WriteSharedObjects();

	// Reads shared objects, these objects have no owner,
	// so must be read separately.
	//    @return - ut::Error if failed.
	Optional<Error> ReadSharedObjects();

	// Searches for the shared parameter in the registry using provided name,
	// then tries to load this parameter.
	//    @param registry - reference to the array of shared cache elements,
	//                      that are waiting to be deserialized.
	//    @param name - name of the serialized shared object.
	//    @param scope_state - state preceding reading of any shared parameter.
	//    @return - ut::Error if failed.
	Optional<Error> LoadSharedObject(Array<InputSharedCacheElement>& registry,
	                                 const String& name,
	                                 const Controller& scope_state);

	// Uses provided id to generate a name of the shared parameter. Calling the
	// same function to generate names both for serialization and deserialization
	// you ensure that parameters would be loaded correctly.
	//    @param id - id of shared parameter.
	//    @return - string containing a generated name.
	String GenerateSharedObjectName(size_t id);

	// Returns a current state.
	Controller SaveState();

	// Loads a provided state.
	void LoadState(const Controller& controller);

	// Modifies @info object according to the provided options.
	// Note that one must restore original @info after reading/writing
	// a node is done.
	//    @param options - set of options.
	//    @return - original (unmodified) copy of the @info object;
	Info ModifyInfo(const SerializationOptions& options);

	// Reads custom data from the binary stream.
	//    @address - pointer to the data to be read.
	//    @granularity - size of one element.
	//    @count - number of elements to write.
	//    @return - ut::Error if failed.
	Optional<Error> ReadBinary(void* address,
	                           size_t granularity,
	                           size_t count);

	// Writes custom data to the binary stream.
	//    @address - pointer to the data to be written.
	//    @granularity - size of one element.
	//    @count - number of elements to write.
	//    @return - ut::Error if failed.
	Optional<Error> WriteBinary(const void* address,
	                            size_t granularity,
	                            size_t count);

	// Helper function to read custom value from the binary stream
	//    @address - pointer to the element to be read
	//    @count - number of elements to read
	//    @return - ut::Error if failed
	template <typename T>
	Optional<Error> ReadBinary(T* address, size_t count)
	{
		return ReadBinary(address, sizeof(T), count);
	}

	// Helper function to write custom value to the binary stream
	//    @address - pointer to the element to be written
	//    @count - number of elements to write
	//    @return - ut::Error if failed
	template <typename T>
	Optional<Error> WriteBinary(const T* address, size_t count)
	{
		return WriteBinary(address, sizeof(T), count);
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
			str.Reset();

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

				str.Append(c);
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

	// Stream cursor, it can differ from the cursor in actual
	// input/output stream. Call Controller::Sync() to synchronize.
	stream::Cursor cursor;

	// Linker helps to read/write links (such as pointers or references).
	SharedPtr<Linker> linker;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//