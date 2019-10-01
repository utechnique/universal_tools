//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_controller.h"
#include "meta/ut_meta_snapshot.h"
#include "meta/ut_meta_linker.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Constructor
//    @param info_copy - copy of the serialization info, that will be
//                       used during serialization and deserialization
Controller::Controller(const Info& info_copy) : info(info_copy)
                                              , mode(empty_mode)
{ }

//----------------------------------------------------------------------------->
// Extracts a custom entity value from the node.
// Note that this information may be absent.
//    @param node_name - name of the child node containing desired value
//    @return - value of desired node, or ut::Error if encountered an error
Result<String, Error> Controller::ExtractTextNodeValue(const Tree<text::Node>& parent_node,
                                                       const String& node_name) const
{
	// search for a desired node
	const Optional< ConstRef< Tree<text::Node> > > find_result = FindTextNode<ConstRef>(parent_node,
	                                                                                    node_name);
	if (!find_result)
	{
		return MakeError(error::not_found);
	}

	// check if value exists
	if (!find_result.Get()->data.value)
	{
		// empty string
		return String();
	}

	// success
	return find_result.Get()->data.value.Get();
}

//----------------------------------------------------------------------------->
// Changes mode to binary input stream
//    @param stream - reference to input stream to read data from
Optional<Error> Controller::SetBinaryInputStream(InputStream& stream)
{
	// set mode and stream
	mode = binary_input_mode;
	io.binary_input = &stream;

	// change cursor position
	Result<stream::Cursor, Error> read_cursor = stream.GetCursor();
	if (!read_cursor)
	{
		return read_cursor.MoveAlt();
	}
	cursor = read_cursor.GetResult();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Changes mode to binary output stream
//    @param stream - reference to output stream to write data to
Optional<Error> Controller::SetBinaryOutputStream(OutputStream& stream)
{
	// set mode and stream
	mode = binary_output_mode;
	io.binary_output = &stream;

	// change cursor position
	Result<stream::Cursor, Error> read_cursor = stream.GetCursor();
	if (!read_cursor)
	{
		return read_cursor.MoveAlt();
	}
	cursor = read_cursor.GetResult();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Changes mode to text input node
//    @param node - reference to the text node to read data from
Optional<Error> Controller::SetTextInputNode(const Tree<text::Node>& node)
{
	mode = text_input_mode;
	io.text_input = &node;
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Changes mode to text output node
//    @param node - reference to the text node to write data to
Optional<Error> Controller::SetTextOutputNode(Tree<text::Node>& node)
{
	mode = text_output_mode;
	io.text_output = &node;
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns current mode
Controller::Mode Controller::GetMode() const
{
	return mode;
}

//----------------------------------------------------------------------------->
// Returns serialization information
Info Controller::GetInfo() const
{
	return info;
}

//----------------------------------------------------------------------------->
// Returns @cursor value.
stream::Cursor Controller::GetCursor() const
{
	return cursor;
}

//----------------------------------------------------------------------------->
// Returns current cursor position of the input/output binary stream.
// If controller is in text mode - returns @cursor value.
// Note that this value can differ from Controller::GetCursor() function result.
Result<stream::Cursor, Error> Controller::GetStreamCursor()
{
	// set stream cursor if controller is in binary mode
	switch (mode)
	{
		case binary_input_mode: return io.binary_input->GetCursor();
		case binary_output_mode: return io.binary_output->GetCursor();
	}

	// return current @cursor position if in text mode
	return cursor;
}

//----------------------------------------------------------------------------->
// Assigns a provided value to the @cursor member variable.
//    @param position - value of the cursor to be set.
//    @param sync - boolean whether to sync binary stream afterwards or not.
//    @return - error if failed.
Optional<Error> Controller::SetCursor(stream::Cursor position, bool sync)
{
	// assign new cursor position
	cursor = position;

	// synchronize stream
	return sync ? Sync() : Optional<Error>();
}

//----------------------------------------------------------------------------->
// Synchronizes binary stream with @cursor position.
// Does nothing if in text mode.
//    @return - error if failed.
Optional<Error> Controller::Sync()
{
	// synchronize binary stream
	if (mode == binary_input_mode)
	{
		return io.binary_input->MoveCursor(cursor);
	}
	else if (mode == binary_output_mode)
	{
		return io.binary_output->MoveCursor(cursor);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Creates a task for linker to write a correct id of the linked
// object (that is defined as a pointer) into the value node.
//    @param parameter - pointer to the parameter representing a link,
//                       (raw pointer, shared/weak ptr, etc.).
//    @param linked_address - adress of the linked object.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteLink(const BaseParameter* parameter,
                                      const void* linked_address)
{
	// check if linker is initialized
	if (!linker)
	{
		return Error(error::fail, "Linker isn't initialized.");
	}

	// save state BEFORE writing a value, so that linker could overwrite it
	Controller state = SaveState();

	// write fake id as a value
	Optional<Error> write_error = WriteValue<SizeType>(0);
	if (write_error)
	{
		return write_error;
	}

	// create linker task to re-write correct id after linking
	return linker->CreateWriteTask(parameter, Move(state), linked_address);
}

//----------------------------------------------------------------------------->
// Creates a task for linker to read an id of the linked
// object (that is defined as a pointer) from the value node,
// and to link it with the provided parameter.
//    @param parameter - pointer to the parameter representing a link,
//                       (raw pointer, shared/weak ptr, etc.).
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadLink(const BaseParameter* parameter)
{
	// check if linker is initialized
	if (!linker)
	{
		return Error(error::fail, "Linker isn't initialized.");
	}

	// read id as a value
	Result<SizeType, Error> read_id_result = ReadValue<SizeType>();
	if (!read_id_result)
	{
		return read_id_result.MoveAlt();
	}

	// create linker task to link parameters after deserialization
	return linker->CreateReadTask(parameter, read_id_result.GetResult());
}

//----------------------------------------------------------------------------->
// Serializes a provided reflective node.
//    @param node - a reference to the ut::meta::Snapshot object to be
//                  serialized, it can be created by calling
//                  ut::meta::Snapshot::Capture() function.
//    @param initialize - this boolean indicates if node will be initialized
//                        with special 'header' information: serialization
//                        info, shared objects, etc.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteNode(Snapshot& node, bool initialize)
{
	// initialize current node
	if (initialize)
	{
		// initialize modules
		Optional<Error> init_error = InitializeNode(node);
		if (init_error)
		{
			return init_error;
		}

		// write initialization data (meta::Info)
		init_error = WriteInitializationData(node);
		if (init_error)
		{
			return init_error;
		}
	}

	// write name, type, id, etc.
	Optional<Error> optional_error = WriteUniformAttributes(node);
	if (optional_error)
	{
		return optional_error;
	}

	// reserve space in binary stream for a size variable (it will be written in the end)
	Result<stream::Cursor, Error> reserve_size_result = ReserveParameterSize();
	if (!reserve_size_result)
	{
		return reserve_size_result.MoveAlt();
	}

	// save parameter ant it's children
	optional_error = WriteParameter(node);
	if (optional_error)
	{
		return optional_error;
	}

	// write parameter size to the reserved space,
	// this affects only a binary variant
	optional_error = WriteParameterSize(reserve_size_result.GetResult());
	if (optional_error)
	{
		return optional_error;
	}

	// finalize current node
	return initialize ? FinalizeNode(node) : Optional<Error>();
}

//----------------------------------------------------------------------------->
// Deserializes a provided reflective node.
//    @param node - a reference to the ut::meta::Snapshot object to be
//                  deserialized, it can be created by calling
//                  ut::meta::Snapshot::Capture() function.
//    @param initialize - this boolean indicates if node will be initialized
//                        with special 'header' information: serialization
//                        info, shared objects, etc.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadNode(Snapshot& node, bool initialize)
{
	// initialize current node
	if (initialize)
	{
		// read initialization data (meta::Info)
		Optional<Error> init_error = ReadInitializationData(node);
		if (init_error)
		{
			return init_error;
		}

		// initialize modules, note that this is done after meta
		// information has been read and processed
		init_error = InitializeNode(node);
		if (init_error)
		{
			return init_error;
		}
	}

	// read name, type, id, etc.
	Optional<String> name, type;
	Optional<Error> read_attributes_error = ReadUniformAttributes(node, name, type);
	if (read_attributes_error)
	{
		return read_attributes_error;
	}

	// read size - this affects only a binary variant
	Result<stream::Cursor, Error> read_size_result = ReadParameterSize();
	if (!read_size_result)
	{
		return read_size_result.MoveAlt();
	}

	// search for a node with a name that matches previously deserialized name
	Result<Ref<Snapshot>, Error> sibling = FindSiblingNode(node, name);
	if (!sibling)
	{
		return SkipParameter(read_size_result.GetResult()); // not fatal, skipping..
	}

	// check types
	if (type)
	{
		Optional<Error> type_check_error = CheckType(sibling.GetResult(), type.Get());
		if (type_check_error)
		{
			return SkipParameter(read_size_result.GetResult()); // not fatal, skipping..
		}
	}

	// read parameter
	Optional<Error> read_param_error = ReadParameter(sibling.GetResult());
	if (read_param_error)
	{
		return read_param_error;
	}

	// finalize current node
	return initialize ? FinalizeNode(node) : Optional<Error>();
}

//----------------------------------------------------------------------------->
// Initializes intermediate modules (such as linker) before reading/writing a node.
//    @param node - reference to the node to initialize.
//    @return - ut::Error if failed.
Optional<Error> Controller::InitializeNode(Snapshot& node)
{
	// create linker
	if (info.HasLinkageInformation())
	{
		linker = new Linker;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Finalizes intermediate modules (such as linker) after a node has been read/written.
//    @param node - reference to the node to finalize.
//    @return - ut::Error if failed.
Optional<Error> Controller::FinalizeNode(Snapshot& node)
{
	// process linkage info
	if (info.HasLinkageInformation())
	{
		// execute linker tasks
		Optional<Error> execute_error = linker->Execute();
		if (execute_error)
		{
			return execute_error;
		}

		// linker isn't needed now
		linker.Reset();
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes ut::meta::Info data, shared objects and linkage data.
//    @param node - reference to the node to initialize.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteInitializationData(Snapshot& node)
{
	// write serialization info
	Optional<Error> write_info_error = WriteInfo();
	if (write_info_error)
	{
		return write_info_error;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads ut::meta::Info and linkage data, creates shared objects.
//    @param node - reference to the node to initialize.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadInitializationData(Snapshot& node)
{
	// read serialization info
	Optional<Error> read_info_error = ReadInfo();
	if (read_info_error)
	{
		return read_info_error;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes attributes that are mandatory for all parameters,
// like name, type, id, etc.
//    @param node - reference to the node that is being serialized.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteUniformAttributes(Snapshot& node)
{
	// write node name
	Optional<Error> optional_error = WriteNodeName(node.data.name);
	if (optional_error)
	{
		return optional_error;
	}

	// write type
	if (info.HasTypeInformation())
	{
		optional_error = WriteAttribute<String>(node.data.parameter->GetTypeName(),
												node_names::skType,
												true);
		if (optional_error)
		{
			return optional_error;
		}
	}

	// write linkage information
	if (info.HasLinkageInformation())
	{
		// create link
		size_t id = linker->GenerateId();
		optional_error = linker->AddLink(node.data.parameter, id);
		if (optional_error)
		{
			return optional_error;
		}

		// write id
		optional_error = WriteAttribute<SizeType>(static_cast<SizeType>(id),
		                                          node_names::skId,
		                                          true);
		if (optional_error)
		{
			return optional_error;
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads attributes that are mandatory for all parameters,
// like name, type, id, etc.
//    @param node - reference to the node that is being deserialized.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadUniformAttributes(Snapshot& node,
                                                  Optional<String>& out_node_name,
                                                  Optional<String>& out_type_name)
{
	// read node name
	Result<Optional<String>, Error> read_name_result = ReadNodeName();
	if (!read_name_result)
	{
		return read_name_result.MoveAlt();
	}
	out_node_name = read_name_result.MoveResult();

	// read type
	if (info.HasTypeInformation())
	{
		Result<String, Error> read_type_result = ReadAttribute<String>(node_names::skType);
		if (!read_type_result)
		{
			return read_type_result.MoveAlt();
		}
		out_type_name = read_type_result.MoveResult();
	}

	// read linkage information
	if (info.HasLinkageInformation())
	{
		// read id
		Result<SizeType, Error> read_id_result = ReadAttribute<SizeType>(node_names::skId);
		if (!read_id_result)
		{
			return read_id_result.MoveAlt();
		}

		// create link
		size_t id = static_cast<size_t>(read_id_result.GetResult());
		Optional<Error> add_link_error = linker->AddLink(node.data.parameter, id);
		if (add_link_error)
		{
			return add_link_error;
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes parameter and all child nodes of this parameter.
//    @param node - reference to the node that is being serialized.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteParameter(Snapshot& node)
{
	// save parameter body
	Optional<Error> save_param_error = node.data.parameter->Save(*this);
	if (save_param_error)
	{
		return save_param_error;
	}

	// write children (this step is recursive)
	Optional<Error> save_children_error = WriteChildNodes(node);
	if (save_children_error)
	{
		return save_children_error;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads parameter and all child nodes of this parameter.
//    @param node - reference to the node that is being deserialized.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadParameter(Snapshot& node)
{
	// here parameter loads it's value and/or attributes that
	// help it to build a new (deserialized) reflection tree, but with empty leaves
	Optional<Error> load_param_error = node.data.parameter->Load(*this);
	if (load_param_error)
	{
		return load_param_error;
	}

	// reflect once again loaded parameter to create those new 'empty leaves'
	node.Empty();
	node.data.parameter->Reflect(node);

	// after the tree structure was restored we can load every leaf separately
	Optional<Error> read_children_error = ReadChildNodes(node);
	if (read_children_error)
	{
		return read_children_error;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Serializes leaves of the provided reflective node.
//    @param node - parent of the leaves to serialize.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteChildNodes(Snapshot& node)
{
	// move one level down into the 'value' node
	// this affects only a text variant, because binary nodes are written
	// sequentially in a binary stream
	if (info.HasValueEncapsulation())
	{
		Optional<Error> dive_down_error = DiveIntoNamedNode(node_names::skValue);
		if (dive_down_error)
		{
			return dive_down_error;
		}
	}

	// write a number of children in the provided node
	const SizeType child_num = static_cast<SizeType>(node.GetNumChildren());
	Optional<Error> child_num_error = WriteNumberOfChildNodes(child_num);
	if (child_num_error)
	{
		return child_num_error;
	}

	// allocate space for child nodes (only text mode is involved)
	Result<size_t, Error> alloc_result = AllocateChildNodes(node.GetNumChildren());
	if (!alloc_result)
	{
		return alloc_result.MoveAlt();
	}

	// save current state
	Controller state = SaveState();

	// iterate child nodes
	const size_t offset = alloc_result.GetResult();
	for (size_t i = 0; i < node.GetNumChildren(); i++)
	{
		// create a new node for serialiation
		Optional<Error> dive_new_node_error = DiveIntoChildNode(i + offset);
		if (dive_new_node_error)
		{
			return dive_new_node_error;
		}

		// write child node
		Optional<Error> save_child_error = WriteNode(node[i], false);
		if (save_child_error)
		{
			return save_child_error;
		}

		// get back to the current node
		LoadState(state);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Deserializes leaves of the provided reflective node.
//    @param node - reference to a parent node.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadChildNodes(Snapshot& node)
{
	// move one level down to the 'value' node
	// this affects only a text variant, because binary nodes are written
	// sequentially in a binary stream
	if (info.HasValueEncapsulation())
	{
		Optional<Error> dive_down_error = DiveIntoNamedNode(node_names::skValue);
		if (dive_down_error)
		{
			return dive_down_error;
		}
	}

	// read a number of children in the serialized node
	Result<size_t, Error> child_num_result = ReadNumberOfChildNodes(node);
	if (!child_num_result)
	{
		return child_num_result.MoveAlt();
	}

	// exit if current node has no children (after initialization)
	if (node.GetNumChildren() == 0)
	{
		return Optional<Error>();
	}

	// save current state
	Controller state = SaveState();

	// iterate child nodes
	for (size_t i = 0; i < child_num_result.GetResult(); i++)
	{
		// create a new child node
		Optional<Error> dive_new_node_error = DiveIntoChildNode(i);
		if (dive_new_node_error)
		{
			return dive_new_node_error;
		}

		// number of children may be not the same in serialized version of the node
		// and in the current one, thus if we want to deserialize a node with an id greater
		// than it's possible for the current version - pick the last id
		size_t node_id = i >= node.GetNumChildren() ? node.GetNumChildren() - 1 : i;

		// read child node
		Optional<Error> load_child_error = ReadNode(node[i], false);
		if (load_child_error)
		{
			return load_child_error;
		}

		// get back to the current node
		LoadState(state);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes a name of the node.
//    @param name - name of the node.
//    @return ut::Error if failed.
Optional<Error> Controller::WriteNodeName(const String& name)
{
	if (mode == binary_output_mode)
	{
		return info.HasBinaryNames() ? WriteBinary<String>(&name, 1) : Optional<Error>();
	}
	else if (mode == text_output_mode)
	{
		io.text_output->data.name = name;
		return Optional<Error>();
	}

	return Error(error::fail, "Invalid mode.");
}

//----------------------------------------------------------------------------->
// Reads a name of the node.
//    @param name - name of the node.
//    @return ut::Error if failed.
Result<Optional<String>, Error> Controller::ReadNodeName()
{
	if (mode == binary_input_mode)
	{
		// if source has no names - return empty container
		if (!info.HasBinaryNames())
		{
			return Optional<String>();
		}

		// otherwise - read a name from stream
		String name;
		Optional<Error> read_error = ReadBinary<String>(&name, 1);
		if (read_error)
		{
			return MakeError(read_error.Move());
		}
		return Optional<String>(name);
	}
	else if (mode == text_input_mode)
	{
		return Optional<String>(io.text_input->data.name);
	}

	return MakeError(Error(error::fail, "Invalid mode."));
}

//----------------------------------------------------------------------------->
// Writes a number of leaves in a node.
//    @param count - number of leaves to be written.
//    @return ut::Error if failed.
Optional<Error> Controller::WriteNumberOfChildNodes(size_t count)
{
	if (mode == binary_output_mode)
	{
		// there is no sense to write a number of children if they have no name
		if (!info.HasBinaryNames())
		{
			return Optional<Error>();
		}

		// write a number of children to the stream
		SizeType converted_count = static_cast<SizeType>(count);
		return WriteBinary<SizeType>(&converted_count, 1);
	}
	else if (mode == text_output_mode)
	{
		// text document retrieves a number of child nodes during a
		// parsing process, so this information must be already known
		return Optional<Error>();
	}

	return Error(error::fail, "Invalid mode.");
}

//----------------------------------------------------------------------------->
// Reads a number of leaves in a node.
//    @param count - number of leaves to be read.
//    @return ut::Error if failed.
Result<size_t, Error> Controller::ReadNumberOfChildNodes(const Snapshot& node)
{
	if (mode == binary_input_mode)
	{
		// if there is no names - there is no number of children, we can only hope
		// that the number of serialized children matches current number of children
		if (!info.HasBinaryNames())
		{
			return node.GetNumChildren();
		}

		// read a number of children from the stream
		SizeType count;
		Optional<Error> read_error = ReadBinary<SizeType>(&count, 1);
		if (read_error)
		{
			return MakeError(read_error.Move());
		}
		return static_cast<size_t>(count);
	}
	else if (mode == text_input_mode)
	{
		// text document retrieves a number of child nodes during a
		// parsing process, so this information is already known
		return io.text_input->GetNumChildren();
	}

	return MakeError(Error(error::fail, "Invalid mode."));
}

//----------------------------------------------------------------------------->
// Allocates space for text child nodes (leaves) so that
// every child node had stable address.
//    @param count - number of child nodes.
//    @return - id of the first child, or error if something went wrong.
Result<size_t, Error> Controller::AllocateChildNodes(size_t count)
{
	size_t id = 0;
	if (mode == text_output_mode)
	{
		// get id of the first leaf
		id = io.text_output->GetNumChildren();

		// preallocate child nodes
		for (size_t i = 0; i < count; i++)
		{
			if (!io.text_output->Add(text::Node()))
			{
				return MakeError(error::out_of_memory);
			}
		}
	}

	// success
	return id;
}

//----------------------------------------------------------------------------->
// Allocates space in the output binary stream for the size variable.
//    @return - position of the size variable in a stream, or
//              ut::Error if something failed.
Result<stream::Cursor, Error> Controller::ReserveParameterSize()
{
	// this function is sensible only for a binary variant
	// and only if parameters can be skipped
	if (mode != binary_output_mode || !SkipIsPossible())
	{
		return static_cast<stream::Cursor>(0);
	}

	// write a number of bytes in the parameter, so that binary stream could skip it
	// if something would go wrong
	Result<stream::Cursor, Error> start_cursor_result = io.binary_output->GetCursor();
	if (!start_cursor_result)
	{
		error::Code error_code = start_cursor_result.GetAlt().GetCode();
		return MakeError(Error(error_code, "Stream doesn't support positioning."));
	}

	// extract starting position
	const stream::Cursor position = start_cursor_result.GetResult();

	// write a size of the parameter
	SizeType parameter_size = 0;
	Optional<Error> save_size_error = WriteBinary<SizeType>(&parameter_size, 1);
	if (save_size_error)
	{
		return MakeError(save_size_error.Move());
	}

	// return a position of the size variable in stream
	return position;
}

//----------------------------------------------------------------------------->
// Writes a size of a parameter to the binary stream.
//    @param start_position - position of the preallocated space for
//                            the size variable in a stream.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteParameterSize(stream::Cursor start_position)
{
	// this function is sensible only for a binary variant
	// and only if parameters have names - so that they could be randomly iterated
	if (mode != binary_output_mode || !SkipIsPossible())
	{
		return Optional<Error>();
	}

	// get a cursor position of the stream
	Result<stream::Cursor, Error> end_position_result = io.binary_output->GetCursor();
	if (!end_position_result)
	{
		error::Code error_code = end_position_result.GetAlt().GetCode();
		return Error(error_code, "Can't get a stream cursor after saving a parameter.");
	}

	// calculate a size of the parameter after it was completely written
	const stream::Cursor end_position = end_position_result.GetResult();
	SizeType parameter_size = static_cast<SizeType>(end_position - start_position);

	// move back cursor position
	Optional<Error> move_back_error = io.binary_output->MoveCursor(start_position);
	if (move_back_error)
	{
		return move_back_error;
	}

	// write parameter size again, but this time it's real
	Optional<Error> save_size_error = WriteBinary<SizeType>(&parameter_size, 1);
	if (save_size_error)
	{
		return save_size_error;
	}

	// move cursor forward
	Optional<Error> move_forward_error = io.binary_output->MoveCursor(end_position);
	if (move_forward_error)
	{
		return move_forward_error;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads a serialized size of a parameter from the binary stream.
//    @return - cursor position of the next parameter or ut::Error if failed.
Result<stream::Cursor, Error> Controller::ReadParameterSize()
{
	// this function is sensible only for binary variant
	if (mode != binary_input_mode || !SkipIsPossible())
	{
		return static_cast<stream::Cursor>(0);
	}

	// get a current stream position so that we could skip current parameter if something
	// would go wrong, this is needed only for a binary variant, because text variant
	// can easily move up and down in a tree
	Result<stream::Cursor, Error> start_pos_result = io.binary_input->GetCursor();
	if (!start_pos_result)
	{
		return MakeError(start_pos_result.MoveAlt());
	}

	// read size
	SizeType parameter_size;
	Optional<Error> read_size_error = ReadBinary<SizeType>(&parameter_size, 1);
	if (read_size_error)
	{
		return MakeError(read_size_error.Move());
	}
	return start_pos_result.GetResult() + static_cast<stream::Cursor>(parameter_size);
}

//----------------------------------------------------------------------------->
// Checks if name of the provided node matches provided name, and if not - searches
// for a sibling node with such a name.
//    @param node - reference to a node to check.
//    @param name - desired node name.
//    @return - a reference to the desired node, or ut::Error if failed.
Result<Ref<Snapshot>, Error> Controller::FindSiblingNode(Snapshot& node, const Optional<String>& name)
{
	// if there is no name - we can't look for the sibling node by name:
	// the only possible candidate is the current node
	if (!name)
	{
		return Ref<Snapshot>(node);
	}

	// check name of the provided node
	if (node.data.name == name.Get())
	{
		return Ref<Snapshot>(node);
	}

	// check siblings of the node
	Snapshot* parent = node.GetParent();
	if (parent)
	{
		Optional< Ref<Snapshot> > find_result = parent->FindChildByName(name.Get());
		if (find_result)
		{
			return find_result.Move();
		}
	}

	// error description
	if (info.HasValueEncapsulation()) // too often for the non-encapsulated nodes
	{
		String error_desc = "Serialization error: Parameter with the name \"";
		error_desc += name.Get() + "\" wasn't found in the registry and was skipped.";
		info.log_signal(error_desc);
	}

	// nothing was found
	return MakeError(error::not_found);
}

//----------------------------------------------------------------------------->
// Checks if provided type names match.
//    @parameter node - reference to the node that contains parameter type.
//    @parameter serialized_type - name of a parameter that was deserialized
//                                 from a stream or a text node.
//    @return - ut::Error if failed.
Optional<Error> Controller::CheckType(const Snapshot& node, const String& serialized_type)
{
	const String current_type = node.data.parameter->GetTypeName();
	if (current_type.CompareCaseInsensitive(serialized_type) != 0)
	{
		// error description
		String error_desc("Serialization error (type mismatch): Parameter with the name \"");
		error_desc += current_type + "\" has type " + current_type + " and serialized type is ";
		error_desc += serialized_type;

		// print error description to log
		info.log_signal(error_desc);

		// exit with an error
		return Error(error::types_not_match, error_desc);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Moves an input stream cursor to the next parameter.
//    @param next - stream position of the next parameter.
//    @return - ut::Error if failed.
Optional<Error> Controller::SkipParameter(stream::Cursor next)
{
	// check if skip is possible
	if (!SkipIsPossible())
	{
		return Error(error::not_supported);
	}

	// this function is sensible only for binary variant
	if (mode != binary_input_mode || !SkipIsPossible())
	{
		return Optional<Error>();
	}

	// skip this parameter
	return io.binary_input->MoveCursor(next);
}

//----------------------------------------------------------------------------->
// Returns 'true' if parameters can be skipped without error if something went wrong.
bool Controller::SkipIsPossible() const
{
	// Skipping is sensible only if we can check type or name, in other words - only 
	// if we can detect that serialized parameter doesn't match a current one
	return mode == text_input_mode || mode == text_output_mode ||
	       info.HasBinaryNames() || info.HasTypeInformation();
}

//----------------------------------------------------------------------------->
// Searches for a desired node inside current text node, and changes input/output
// source/target to this value node.
//    @param name - name of the node.
//    @return - ut::Error if failed.
Optional<Error> Controller::DiveIntoNamedNode(const String& name)
{
	// this function is sensible only for text variant
	if (mode == text_output_mode)
	{
		// get reference to the parent node
		Tree<text::Node>& parent = *io.text_output;

		// check if value node exists
		Optional< Ref< Tree<text::Node> > > find_result = FindTextNode<Ref>(parent, name);
		if (find_result)
		{
			// use existing node as an input source
			Tree<text::Node>& value_node = find_result.Get();
			io.text_output = &value_node;
		}
		else
		{
			// create a new node
			Tree<text::Node> new_node;
			new_node.data.name = name;
			if (!parent.Add(Move(new_node)))
			{
				return Error(error::out_of_memory);
			}

			// set new input source
			io.text_output = &parent.GetLastChild();
		}
	}
	else if (mode == text_input_mode)
	{
		// get reference to the parent node
		const Tree<text::Node>& parent = *io.text_input;

		// search for the value node
		Optional< ConstRef< Tree<text::Node> > > find_result = FindTextNode<ConstRef>(parent, name);

		// check if value node exists
		if (!find_result)
		{
			// error description
			String error_desc("Serialization error: Parameter with the name \"");
			error_desc += parent.data.name + "\" has no \"" + name + "\" node.";

			// print error description to log
			info.log_signal(error_desc);

			// exit with error
			return Error(error::not_found, error_desc);
		}

		// set new output target
		const Tree<text::Node>& value_node = find_result.Get();
		io.text_input = &value_node;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes serialization information about the current serialization node.
// Information is written to the special attribute node (it's name is 
// defined here: ut::meta::node_names::skInfo).
//     @return - ut::Error if failed.
Optional<Error> Controller::WriteInfo()
{
	// save state
	Controller state = SaveState();

	// get varsion and flags before writing
	Info::Version version = info.GetVersion();
	Info::Flag flags = info.GetFlags();

	// info is always written in little-endian mode
	info.SetEndianness(endian::little);

	// create 'info' node
	Optional<Error> optional_error = DiveIntoNamedNode(node_names::skInfo);
	if (optional_error)
	{
		return optional_error;
	}

	// write version
	optional_error = WriteAttribute(version, node_names::skVersion, true);
	if (optional_error)
	{
		return optional_error;
	}

	// write flags
	optional_error = WriteAttribute(flags, node_names::skFlags, true);
	if (optional_error)
	{
		return optional_error;
	}

	// get back to the parent node
	LoadState(state);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads serialization information about the current serialization node
// and applies correct settings to the controller.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadInfo()
{
	// save state
	Controller state = SaveState();

	// info is always written in little-endian mode
	info.SetEndianness(endian::little);

	// create 'info' node
	Optional<Error> optional_error = DiveIntoNamedNode(node_names::skInfo);
	if (optional_error)
	{
		return optional_error;
	}

	// read version
	Result<Info::Version, Error> version = ReadAttribute<Info::Version>(node_names::skVersion);
	if (!version)
	{
		return version.MoveAlt();
	}

	// read flags
	Result<Info::Flag, Error> flags = ReadAttribute<Info::Flag>(node_names::skFlags);
	if (!flags)
	{
		return flags.MoveAlt();
	}

	// get back to the parent node
	LoadState(state);
	
	// set controller flags and version
	info.SetVersion(version.GetResult());
	info.SetFlags(flags.GetResult());

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Dives into a text node that is defined by an id, and changes
// input/output source/target to this node.
//    @parameter id - id of the child node to dive in.
//    @return - ut::Error if failed.
Optional<Error> Controller::DiveIntoChildNode(size_t id)
{
	// this function is sensible only for text variant
	if (mode == text_output_mode)
	{
		// get reference to the parent node
		Tree<text::Node>& parent = *io.text_output;

		// check range
		if (id >= parent.GetNumChildren())
		{
			return Error(error::out_of_bounds);
		}

		// set new output target
		io.text_output = &parent[id];
	}
	else if (mode == text_input_mode)
	{
		// get reference to the parent node
		const Tree<text::Node>& parent = *io.text_input;

		// check range
		if (id >= parent.GetNumChildren())
		{
			return Error(error::out_of_bounds);
		}

		// set new output target
		io.text_input = &parent[id];
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns a current state.
Controller Controller::SaveState()
{
	return *this;
}

//----------------------------------------------------------------------------->
// Loads a provided state.
void Controller::LoadState(const Controller& controller)
{
	*this = controller;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//