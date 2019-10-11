//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/linkage/ut_meta_linker.h"
#include "meta/ut_meta_controller.h"
#include "meta/ut_meta_snapshot.h"
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
// Synchronizes @cursor position with binary stream.
// Does nothing if in text mode.
//    @return - error if failed.
Optional<Error> Controller::SyncWithStream()
{
	// synchronize binary stream
	if (mode == binary_input_mode)
	{
		Result<stream::Cursor, Error> stream_cursor = io.binary_input->GetCursor();
		if (!stream_cursor)
		{
			return stream_cursor.MoveAlt();
		}
		cursor = stream_cursor.GetResult();
	}
	else if (mode == binary_output_mode)
	{
		Result<stream::Cursor, Error> stream_cursor = io.binary_output->GetCursor();
		if (!stream_cursor)
		{
			return stream_cursor.MoveAlt();
		}
		cursor = stream_cursor.GetResult();
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
	UniquePtr<LinkTask> write_task(new WriteLinkTask(Move(state), linked_address));
	return linker->AddTask(Move(write_task));
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
	UniquePtr<LinkTask> read_task(new ReadLinkTask(parameter, read_id_result.GetResult()));
	return linker->AddTask(Move(read_task));
}

//----------------------------------------------------------------------------->
// Creates a task for linker to read an id of the linked
// shared object from the value node, and to link it with
// the provided parameter.
//    @param parameter - pointer to the parameter representing a link,
//                       (shared ptr).
//    @param ptr - shared pointer to the holder of the SharedPtr object.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadSharedLink(const BaseParameter* parameter,
                                           const SharedPtr<SharedPtrHolderBase>& ptr)
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

	// let the linker know about the existance of the linked shared object
	Optional<Error> cache_error = linker->RegisterInputSharedObject(ptr, read_id_result.GetResult());
	if (cache_error)
	{
		return cache_error;
	}

	// create linker task to link parameters after deserialization
	UniquePtr<LinkTask> read_task(new ReadSharedPtrLinkTask(parameter, read_id_result.GetResult()));
	return linker->AddTask(Move(read_task));
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

	// save state
	Controller state = SaveState();

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

	// get back to the current node
	LoadState(state); // here @cursor becomes '0' again
	SyncWithStream(); // here @cursor is synchronized with current stream position

	// finalize current node
	return initialize ? FinalizeNode(node) : Optional<Error>();
}

//----------------------------------------------------------------------------->
// Deserializes a provided reflective node.
//    @param node - a reference to the ut::meta::Snapshot object to be
//                  deserialized, it can be created by calling
//                  ut::meta::Snapshot::Capture() function;
//                  this parameter can be empty if @skip_loading is 'true'.
//    @param initialize - this boolean indicates if node will be initialized
//                        with special 'header' information: serialization
//                        info, shared objects, etc; if this parameter is 'true' -
//                        then @skip_loading parameter must be 'false'
//    @param skip_loading - this boolean indicates if node's body must be skipped
//                          and only uniform data is to be read, @node parameter
//                          can be empty in this case.
//    @return - ut::meta::Controller::Uniform object describing a node
//              or ut::Error if failed.
Result<Controller::Uniform, Error> Controller::ReadNode(Optional< Ref<Snapshot> > node,
                                                        bool initialize,
                                                        bool skip_loading)
{
	// validate parameters
	if (!node && !skip_loading)
	{
		return MakeError(Error(error::invalid_arg, "Node can't be null."));
	}
	else if (initialize && skip_loading)
	{
		return MakeError(Error(error::invalid_arg, "Initializing and skipping simultaneously is forbidden."));
	}

	// initialize current node
	if (initialize)
	{
		// read initialization data (meta::Info)
		Optional<Error> init_error = ReadInitializationData(node.Get());
		if (init_error)
		{
			return MakeError(init_error.Move());
		}

		// initialize modules, note that this is done after meta
		// information has been read and processed
		init_error = InitializeNode(node.Get());
		if (init_error)
		{
			return MakeError(init_error.Move());
		}
	}

	// save state
	Controller state = SaveState();

	// read name, type, id, etc.
	Result<Controller::Uniform, Error> uniforms_result = ReadUniformAttributes();
	if (!uniforms_result)
	{
		return MakeError(uniforms_result.MoveAlt());
	}
	Controller::Uniform uniforms(uniforms_result.MoveResult());

	// read size - this affects only a binary variant
	Result<stream::Cursor, Error> read_size_result = ReadParameterSize();
	if (!read_size_result)
	{
		return MakeError(read_size_result.MoveAlt());
	}

	// skip this node in case we are forbidden to move further
	if (skip_loading)
	{
		Optional<Error> skip_error = SkipParameter(read_size_result.GetResult());
		if (skip_error)
		{
			return MakeError(skip_error.Move());
		}
		return uniforms;
	}

	// search for a node with a name that matches previously deserialized name
	Result<Ref<Snapshot>, Error> sibling = FindSiblingNode(node.Get(), uniforms.name);
	if (!sibling) // not fatal, skipping..
	{
		Optional<Error> skip_error = SkipParameter(read_size_result.GetResult());
		if (skip_error)
		{
			return MakeError(skip_error.Move());
		}
		return uniforms;
	}

	// check types
	if (uniforms.type)
	{
		Optional<Error> type_check_error = CheckType(sibling.GetResult(), uniforms.type.Get());
		if (type_check_error) // not fatal, skipping..
		{
			Optional<Error> skip_error = SkipParameter(read_size_result.GetResult());
			if (skip_error)
			{
				return MakeError(skip_error.Move());
			}
			return uniforms;
		}
	}

	// create link
	if (uniforms.id)
	{
		size_t id = static_cast<size_t>(uniforms.id.Get());
		Optional<Error> add_link_error = linker->AddLink(node.Get()->data.parameter, id);
		if (add_link_error)
		{
			return MakeError(add_link_error.Move());
		}
	}

	// read parameter
	Optional<Error> read_param_error = ReadParameter(sibling.GetResult());
	if (read_param_error)
	{
		return MakeError(read_param_error.Move());
	}

	// get back to the current node
	LoadState(state);

	// finalize current node
	if (initialize)
	{
		Optional<Error> finalize_error = FinalizeNode(node.Get());
		if (finalize_error)
		{
			return MakeError(finalize_error.Move());
		}
	}

	// success
	return uniforms;
}

//----------------------------------------------------------------------------->
// Adds unique shared parameter.
//    @param ptr - shared pointer to the holder of the SharedPtr object.
//    @param address - address of the shared object.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteSharedObject(const SharedPtr<SharedPtrHolderBase>& ptr,
                                              const void* address)
{
	// check if linker exists
	if (!linker)
	{
		return Error(error::fail, "Linker isn't initialized.");
	}

	// register object
	return linker->CacheOutputSharedObject(ptr, address);
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
		// write/read shared objects before executing linker tasks
		bool output_mode = mode == binary_output_mode || mode == text_output_mode;
		const Optional<Error> shared_error = output_mode ? WriteSharedObjects() : ReadSharedObjects();
		if(shared_error)
		{
			return shared_error;
		}

		// execute linker tasks
		const Optional<Error> execute_error = linker->Execute();
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
//    @return - meta::Controller::Uniform object ot ut::Error if failed.
Result<Controller::Uniform, Error> Controller::ReadUniformAttributes()
{
	// result of the function
	Uniform out;

	// read node name
	Result<Optional<String>, Error> read_name_result = ReadNodeName();
	if (!read_name_result)
	{
		return MakeError(read_name_result.MoveAlt());
	}
	out.name = read_name_result.MoveResult();

	// read type
	if (info.HasTypeInformation())
	{
		Result<String, Error> read_type_result = ReadAttribute<String>(node_names::skType);
		if (!read_type_result)
		{
			return MakeError(read_type_result.MoveAlt());
		}
		out.type = read_type_result.MoveResult();
	}

	// read linkage information
	if (info.HasLinkageInformation())
	{
		// read id
		Result<SizeType, Error> read_id_result = ReadAttribute<SizeType>(node_names::skId);
		if (!read_id_result)
		{
			return MakeError(read_id_result.MoveAlt());
		}
		out.id = read_id_result.MoveResult();
	}

	// success
	return out;
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
		Result<Controller::Uniform, Error> read_result = ReadNode(Ref<Snapshot>(node[i]), false);
		if (!read_result)
		{
			return read_result.MoveAlt();
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
		info.LogMessage(error_desc);
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
		info.LogMessage(error_desc);

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
	return SetCursor(next, true);
}

//----------------------------------------------------------------------------->
// Returns 'true' if parameters can be skipped without error if something went wrong.
bool Controller::SkipIsPossible() const
{
	// Skipping is sensible only if we can check type or name, in other words - only 
	// if we can detect that serialized parameter doesn't match a current one
	return mode == text_input_mode || mode == text_output_mode ||
	       info.HasBinaryNames() || info.HasTypeInformation() || info.HasLinkageInformation();
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
			info.LogMessage(error_desc);

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
// Writes shared objects, these objects have no owner
// (and thus must be written separately).
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteSharedObjects()
{
	// save state
	Controller state = SaveState();

	// create 'shared_objects' node
	Optional<Error> optional_error = DiveIntoNamedNode(node_names::skSharedObjects);
	if (optional_error)
	{
		return optional_error;
	}

	// save position of the 'count' variable
	Result<stream::Cursor, Error> count_cursor = GetCursor();
	if (!count_cursor)
	{
		return count_cursor.MoveAlt();
	}

	// write number of parameters, whis value will be rewritten
	SizeType count = 0;
	optional_error = WriteAttribute(count, node_names::skCount, true);
	if (optional_error)
	{
		return optional_error;
	}

	// save state
	Controller local_state = SaveState();

	// cycle repeats untill all circular links are cached and serialized 
	while (true)
	{
		// grab shared objects ready for serialization from linker
		Array<OutputSharedCacheElement> shared_objects = linker->MoveOutputSharedCache();
		if (shared_objects.GetNum() == 0)
		{
			break;
		}

		// write parameters
		for (size_t i = 0; i < shared_objects.GetNum(); i++)
		{
			// generate correct node name
			const String node_name = GenerateSharedObjectName(count);
			optional_error = DiveIntoNamedNode(node_name);
			if (optional_error)
			{
				return optional_error;
			}

			// save shared parameter
			// note that this step can provoke linker to cache new shared links,
			// that's why we have 'while (true)' loop untill all new links are gone
			optional_error = shared_objects[i].ptr->Save(*this, node_name);
			if (optional_error)
			{
				return optional_error;
			}

			// accumulate count value
			count++;

			// get back to the parent node
			LoadState(local_state);
		}
	}

	// save cursor position (where all shared parameters are already written)
	Result<stream::Cursor, Error> current_cursor = GetCursor();
	if (!current_cursor)
	{
		return current_cursor.MoveAlt();
	}

	// write final number of parameters
	SetCursor(count_cursor.GetResult(), true);
	optional_error = WriteAttribute(count, node_names::skCount, true);
	if (optional_error)
	{
		return optional_error;
	}

	// set cursor back (where all shared parameters are already written)
	SetCursor(current_cursor.GetResult(), true);

	// get back to the upper most node
	LoadState(state);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Reads shared objects, these objects have no owner,
// so must be read separately.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadSharedObjects()
{
	// save state
	Controller state = SaveState();

	// dive into the 'shared_objects' node
	Optional<Error> optional_error = DiveIntoNamedNode(node_names::skSharedObjects);
	if (optional_error)
	{
		return optional_error;
	}

	// read number of parameters
	Result<Info::Version, Error> count = ReadAttribute<SizeType>(node_names::skCount);
	if (!count)
	{
		return count.MoveAlt();
	}

	// save state
	Controller local_state = SaveState();

	// grab a portion of newly registered shared parameters
	Array<InputSharedCacheElement> registry = linker->MovePreliminarySharedCache();

	// read all shared objects
	for (size_t i = 0; i < count.GetResult(); i++)
	{
		// generate correct node name
		const String node_name = GenerateSharedObjectName(i);
		optional_error = DiveIntoNamedNode(node_name);
		if (optional_error)
		{
			return optional_error;
		}

		// load shared object
		Optional<Error> load_error = LoadSharedObject(registry, node_name, local_state);
		if (load_error)
		{
			return load_error;
		}
	}

	// get back to the parent node
	LoadState(state);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Searches for the shared parameter in the registry using provided name,
// then tries to load this parameter.
//    @param registry - reference to the array of shared cache elements,
//                      that are waiting to be deserialized.
//    @param name - name of the serialized shared object.
//    @param scope_state - state preceding reading of any shared parameter.
//    @return - ut::Error if failed.
Optional<Error> Controller::LoadSharedObject(Array<InputSharedCacheElement>& registry,
                                             const String& name,
                                             const Controller& scope_state)
{
	// read information (name, type, id) and nothing else
	Controller node_state = SaveState();
	Result<Uniform, Error> uniform = ReadNode(Optional< Ref<Snapshot> >(),
	                                          false, true);
	if (!uniform)
	{
		return uniform.MoveAlt();
	}

	// id must be present
	if (!uniform.GetResult().id)
	{
		String error_desc = "Shared object has no \"id\" value.";
		info.LogMessage(error_desc);
		return Error(error::fail, error_desc);
	}

	// iterate all registry entries to find id match
	for (size_t i = registry.GetNum(); i-- > 0;)
	{
		// check if id matches
		if (registry[i].id != static_cast<size_t>(uniform.GetResult().id.Get()))
		{
			continue;
		}

		// get back to the state where node is not read yet
		LoadState(node_state);
		Sync(); // stream cursor is farther after reading uniforms, must be adjusted

		// load shared object
		Optional<Error> load_error = registry[i].ptr->Load(*this, name);
		if (load_error)
		{
			return load_error;
		}

		// add deserialized parameter to the cache
		Optional<Error> cache_error = linker->CacheInputSharedObject(registry[i].ptr,
			                                                         registry[i].id);
		if (cache_error)
		{
			return cache_error;
		}

		// remove enty from the registry - it's already loaded and ready to be linked with
		registry.Remove(i);

		// new preliminary links can occur while loading a shared object, thus it's essential
		// to add these new entries to the current registry so that further shared objects
		// (that are deeper than 1 level in linking hierarchy) could be deserialized
		Array<InputSharedCacheElement> new_registry_entries = linker->MovePreliminarySharedCache();
		for (size_t j = 0; j < new_registry_entries.GetNum(); j++)
		{
			registry.Add(Move(new_registry_entries[j]));
		}

		// exit the loop
		break;
	}

	// save current binary stream position
	Result<stream::Cursor, Error> stream_pos = GetStreamCursor();
	if (!stream_pos)
	{
		return stream_pos.MoveAlt();
	}

	// load state that precedes reading of any shared parameter
	LoadState(scope_state);

	// synchronize stream postion with controller's one
	SetCursor(stream_pos.GetResult());

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Uses provided id to generate a name of the shared parameter. Calling the
// same function to generate names both for serialization and deserialization
// you ensure that parameters would be loaded correctly.
//    @param id - id of shared parameter.
//    @return - string containing a generated name.
String Controller::GenerateSharedObjectName(size_t id)
{
	return String("sh") + Print<SizeType>(static_cast<SizeType>(id));
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