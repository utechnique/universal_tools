//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/linkage/ut_meta_linker.h"
#include "meta/ut_meta_controller.h"
#include "meta/ut_meta_snapshot.h"
#include "encryption/ut_base64.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Constructor
//    @param info_copy - copy of the serialization info, that will be
//                       used during serialization and deserialization
Controller::Controller(const Info& info_copy) : info(info_copy)
                                              , mode(Mode::empty)
{ }

//----------------------------------------------------------------------------->
// Extracts a custom entity value from the node.
// Note that this information may be absent.
//    @param node_name - name of the child node containing desired value
//    @return - value of desired node, or nothing if encountered an error
Optional<String> Controller::ExtractTextNodeValue(const Tree<text::Node>& parent_node,
                                                  const String& node_name) const
{
	// search for a desired node
	const Optional< ConstRef< Tree<text::Node> > > find_result = FindTextNode<ConstRef>(parent_node,
	                                                                                    node_name);
	if (!find_result)
	{
		return Optional<String>();
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
	mode = Mode::binary_input;
	io.binary_input = &stream;

	// change cursor position
	Result<stream::Cursor, Error> read_cursor = stream.GetCursor();
	if (!read_cursor)
	{
		return read_cursor.MoveAlt();
	}
	cursor = read_cursor.Get();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Changes mode to binary output stream
//    @param stream - reference to output stream to write data to
Optional<Error> Controller::SetBinaryOutputStream(OutputStream& stream)
{
	// set mode and stream
	mode = Mode::binary_output;
	io.binary_output = &stream;

	// change cursor position
	Result<stream::Cursor, Error> read_cursor = stream.GetCursor();
	if (!read_cursor)
	{
		return read_cursor.MoveAlt();
	}
	cursor = read_cursor.Get();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Changes mode to text input node
//    @param node - reference to the text node to read data from
Optional<Error> Controller::SetTextInputNode(const Tree<text::Node>& node)
{
	mode = Mode::text_input;
	io.text_input = &node;
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Changes mode to text output node
//    @param node - reference to the text node to write data to
Optional<Error> Controller::SetTextOutputNode(Tree<text::Node>& node)
{
	mode = Mode::text_output;
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
		case Mode::binary_input: return io.binary_input->GetCursor();
		case Mode::binary_output: return io.binary_output->GetCursor();
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
	if (mode == Mode::binary_input)
	{
		return io.binary_input->MoveCursor(cursor);
	}
	else if (mode == Mode::binary_output)
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
	if (mode == Mode::binary_input)
	{
		Result<stream::Cursor, Error> stream_cursor = io.binary_input->GetCursor();
		if (!stream_cursor)
		{
			return stream_cursor.MoveAlt();
		}
		cursor = stream_cursor.Get();
	}
	else if (mode == Mode::binary_output)
	{
		Result<stream::Cursor, Error> stream_cursor = io.binary_output->GetCursor();
		if (!stream_cursor)
		{
			return stream_cursor.MoveAlt();
		}
		cursor = stream_cursor.Get();
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
	UniquePtr<LinkTask> read_task(new ReadLinkTask(parameter, read_id_result.Get()));
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
	Optional<Error> cache_error = linker->RegisterInputSharedObject(ptr, read_id_result.Get());
	if (cache_error)
	{
		return cache_error;
	}

	// create linker task to link parameters after deserialization
	UniquePtr<LinkTask> read_task(new ReadSharedPtrLinkTask(parameter, read_id_result.Get()));
	return linker->AddTask(Move(read_task));
}

//----------------------------------------------------------------------------->
// Creates a task for linker to read an id of the linked
// shared object from the value node, and to link it with
// the provided parameter.
//    @param parameter - pointer to the parameter representing a link,
//                       (weak ptr).
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadWeakLink(const BaseParameter* parameter)
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
	UniquePtr<LinkTask> read_task(new ReadSharedPtrLinkTask(parameter, read_id_result.Get()));
	return linker->AddTask(Move(read_task));
}

//----------------------------------------------------------------------------->
// Serializes a provided reflective node.
//    @param node - a reference to the ut::meta::Snapshot object to be
//                  serialized, it can be created by calling
//                  ut::meta::Snapshot::Capture() function.
//    @param options - options specifying how exactly @node is supposed to
//                     be written.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteNode(Snapshot& node,
                                      const SerializationOptions& options)
{
	// initialize current node
	if (options.initialize)
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

	// remember cursor before writing a parameter
	Result<stream::Cursor, Error> start_cursor = GetStreamCursor();
	if (!start_cursor)
	{
		return start_cursor.MoveAlt();
	}

	// save state
	Controller state = SaveState();

	// modify info object according to the provided options
	Info original_info = ModifyInfo(options);

	// write name, type, id, size etc.
	Result<Optional<stream::Cursor>, Error> uniform_result = WriteUniformAttributes(node);
	if (!uniform_result)
	{
		return uniform_result.MoveAlt();
	}

	// set original info object back
	info = original_info;

	// reserve space in binary stream for a size variable (it will be written in the end)
	Optional<stream::Cursor> size_offset(uniform_result.Move());

	// save parameter ant it's children
	Optional<Error> optional_error = WriteParameter(node, start_cursor.Get());
	if (optional_error)
	{
		return optional_error;
	}

	// write parameter size to the reserved space,
	// this affects only a binary variant
	if (size_offset)
	{
		optional_error = WriteParameterSize(size_offset);
		if (optional_error)
		{
			return optional_error;
		}
	}

	// get back to the current node
	LoadState(state); // here @cursor becomes '0' again
	SyncWithStream(); // here @cursor is synchronized with current stream position

	// finalize current node
	return options.initialize ? FinalizeNode(node) : Optional<Error>();
}

//----------------------------------------------------------------------------->
// Deserializes a provided reflective node.
//    @param node - a reference to the ut::meta::Snapshot object to be
//                  deserialized, it can be created by calling
//                  ut::meta::Snapshot::Capture() function;
//                  this parameter can be empty if @skip_loading is 'true'.
//    @param options - options specifying how exactly @node is supposed to
//                     be read.
//    @return - ut::meta::Controller::Uniform object describing a node
//              or ut::Error if failed.
Result<Controller::Uniform, Error> Controller::ReadNode(Optional<Snapshot&> node,
                                                        const SerializationOptions& options)
{
	// validate parameters
	if (!node && !options.only_uniforms)
	{
		return MakeError(error::invalid_arg, "Node can't be null.");
	}
	else if (options.initialize && options.only_uniforms)
	{
		return MakeError(error::invalid_arg, "Initializing and skipping simultaneously is forbidden.");
	}

	// initialize current node
	if (options.initialize)
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

	// remember cursor before writing a parameter
	Result<stream::Cursor, Error> start_cursor = GetStreamCursor();
	if (!start_cursor)
	{
		return MakeError(start_cursor.MoveAlt());
	}

	// save state
	Controller state = SaveState();

	// modify info object according to the provided options
	Info original_info = ModifyInfo(options);

	// read name, type, id, etc.
	Result<Controller::Uniform, Error> uniforms_result = ReadUniformAttributes();
	if (!uniforms_result)
	{
		return MakeError(uniforms_result.MoveAlt());
	}
	Controller::Uniform uniforms(uniforms_result.Move());
	
	// skip this node in case we are forbidden to move further
	if (options.only_uniforms)
	{
		Optional<Error> skip_error = SkipParameter(uniforms.next);
		if (skip_error)
		{
			return MakeError(skip_error.Move());
		}
	}

	// set original info object back
	info = original_info;

	// exit if only uniforms had to be read here
	if (options.only_uniforms)
	{
		return uniforms;
	}

	// search for a node with a name that matches previously deserialized name
	Optional<Snapshot&> sibling = FindSiblingNode(node.Get(), uniforms.name);
	if (!sibling) // not fatal, skipping..
	{
		Optional<Error> skip_error = SkipParameter(uniforms.next);
		if (skip_error)
		{
			return MakeError(skip_error.Move());
		}
		return uniforms;
	}

	// check types
	if (uniforms.type)
	{
		Optional<Error> type_check_error = CheckType(sibling.Get(), uniforms.type.Get());
		if (type_check_error) // not fatal, skipping..
		{
			Optional<Error> skip_error = SkipParameter(uniforms.next);
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
		Optional<Error> add_link_error = linker->AddLink(node->data.parameter, id);
		if (add_link_error)
		{
			return MakeError(add_link_error.Move());
		}
	}

	// read parameter
	Optional<Error> read_param_error = ReadParameter(sibling.Get(), start_cursor.Get());
	if (read_param_error)
	{
		return MakeError(read_param_error.Move());
	}

	// get back to the current node
	LoadState(state);

	// finalize current node
	if (options.initialize)
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
// Reads binary data from the current value node.
//    @param dst - pointer to the destination binary data.
//    @param size - size of the data in bytes.
//    @param granularity - size of one element in the provided data.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadBinaryValue(void* dst,
                                            Controller::SizeType size,
                                            Controller::SizeType granularity)
{
	if (size % granularity != 0)
	{
		return Error(error::invalid_arg, "Invalid granularity");
	}

	const size_t element_count = size / granularity;

	if (mode == Mode::binary_input)
	{
		Optional<Error> read_error = ReadBinary(dst, granularity, element_count);
		if (read_error)
		{
			return Error(read_error.Move());
		}
	}
	else if (mode == Mode::text_input) // read text form
	{
		Result<String, Error> extraction_result = ReadValue<String>();
		if (!extraction_result)
		{
			return Error(extraction_result.MoveAlt());
		}

		Array<ut::byte> decoded_data = DecodeBase64(extraction_result.Get());
		if (decoded_data.GetSize() != size)
		{
			return Error(error::out_of_bounds);
		}

		const endianness::Order order = info.GetEndianness();
		if (order == endianness::GetNative())
		{
			memory::Copy(dst, decoded_data.GetAddress(), size);
		}
		else
		{
			BinaryStream binary_stream;
			binary_stream.SetBuffer(ut::Move(decoded_data));
			Optional<Error> read_error = order == endianness::Order::little ?
			                             endianness::Read<endianness::Order::little>(binary_stream, dst, granularity, element_count) :
			                             endianness::Read<endianness::Order::big>(binary_stream, dst, granularity, element_count);
			if (read_error)
			{
				return read_error;
			}
		}
	}
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes provided binary data to the current value node.
//    @param data - pointer to the binary data to be written.
//    @param size - size of the data in bytes.
//    @param granularity - size of one element in the provided data.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteBinaryValue(const void* data,
                                             Controller::SizeType size,
                                             Controller::SizeType granularity)
{
	if (size % granularity != 0)
	{
		return Error(error::invalid_arg, "Invalid granularity");
	}

	const size_t element_count = size / granularity;

	if (mode == Mode::text_output)
	{
		// resolve endianness
		BinaryStream binary_stream;
		const endianness::Order order = info.GetEndianness();
		if (order != endianness::GetNative())
		{
			Optional<Error> write_error = order == endianness::Order::little ?
			                              endianness::Write<endianness::Order::little>(binary_stream, data, granularity, element_count) :
			                              endianness::Write<endianness::Order::big>(binary_stream, data, granularity, element_count);
			if (write_error)
			{
				return write_error;
			}

			ut::Result<const void*, ut::Error> stream_data = binary_stream.GetData();
			if (!stream_data)
			{
				return ut::Error(stream_data.MoveAlt());
			}

			data = stream_data.Get();
		}

		String base64 = EncodeBase64(data, static_cast<size_t>(size));
		return WriteValue<String>(base64);
	}
	else
	{
		Optional<Error> write_error = WriteBinary(data, granularity, element_count);
		if (write_error)
		{
			return write_error;
		}
	}
	return Optional<Error>();
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
		linker = MakeShared<Linker>();
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
		bool output_mode = mode == Mode::binary_output || mode == Mode::text_output;
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
// like name, type, id, size etc.
//    @param node - reference to the node that is being serialized.
//    @return - Optional stream offset to the size variable
//              or ut::Error if failed.
Result<Optional<stream::Cursor>, Error> Controller::WriteUniformAttributes(Snapshot& node)
{
	Optional<Error> optional_error;

	// generate unique linkage id, and add it to the linker
	size_t link_id;
	if (info.HasLinkageInformation())
	{
		if (mode == Mode::binary_output)
		{
			Result<stream::Cursor, Error> read_cursor = io.binary_output->GetCursor();
			if (!read_cursor)
			{
				return MakeError(read_cursor.MoveAlt());
			}
			link_id = read_cursor.Get();
		}
		else
		{
			link_id = linker->GenerateId();
		}

		// create link, note that id is an offset in bytes from the beginning of
		// the stream in binary mode and a custom unique value for the text mode
		optional_error = linker->AddLink(node.data.parameter, link_id);
		if (optional_error)
		{
			return MakeError(optional_error.Move());
		}
	}

	// write node name
	optional_error = WriteNodeName(node.data.name);
	if (optional_error)
	{
		return MakeError(optional_error.Move());
	}

	// write type
	if (info.HasTypeInformation())
	{
		optional_error = WriteAttribute<String>(node.data.parameter->GetTypeName(),
												node_names::skType,
												true);
		if (optional_error)
		{
			return MakeError(optional_error.Move());
		}
	}

	// write linkage information (only text mode)
	if (info.HasLinkageInformation() && mode == Mode::text_output)
	{
		optional_error = WriteAttribute<SizeType>(static_cast<SizeType>(link_id),
			                                        node_names::skId,
			                                        true);
		if (optional_error)
		{
			return MakeError(optional_error.Move());
		}
	}

	// write size
	Result<Optional<stream::Cursor>, Error> reserve_size_result = ReserveParameterSize();
	if (!reserve_size_result)
	{
		return MakeError(reserve_size_result.MoveAlt());
	}

	// success
	return reserve_size_result.Move();
}

//----------------------------------------------------------------------------->
// Reads attributes that are mandatory for all parameters,
// like name, type, id, etc.
//    @return - meta::Controller::Uniform object ot ut::Error if failed.
Result<Controller::Uniform, Error> Controller::ReadUniformAttributes()
{
	// result of the function
	Uniform out;

	// linkage id can be easily calculated for the binary mode
	if (info.HasLinkageInformation() && mode == Mode::binary_input)
	{
		Result<stream::Cursor, Error> read_cursor = io.binary_input->GetCursor();
		if (!read_cursor)
		{
			return MakeError(read_cursor.MoveAlt());
		}
		out.id = static_cast<SizeType>(read_cursor.Get());
	}

	// read node name
	Result<Optional<String>, Error> read_name_result = ReadNodeName();
	if (!read_name_result)
	{
		return MakeError(read_name_result.MoveAlt());
	}
	out.name = read_name_result.Move();

	// read type
	if (info.HasTypeInformation())
	{
		Result<String, Error> read_type_result = ReadAttribute<String>(node_names::skType);
		if (!read_type_result)
		{
			return MakeError(read_type_result.MoveAlt());
		}
		out.type = read_type_result.Move();
	}

	// read linkage information
	if (info.HasLinkageInformation() && mode == Mode::text_input)
	{
		Result<SizeType, Error> read_id_result = ReadAttribute<SizeType>(node_names::skId);
		if (!read_id_result)
		{
			return MakeError(read_id_result.MoveAlt());
		}
		out.id = read_id_result.Move();
	}

	// read parameter size, size variable must be the last one of uniforms
	Result<stream::Cursor, Error> read_size_result = ReadParameterSize();
	if (!read_size_result)
	{
		return MakeError(read_size_result.MoveAlt());
	}
	out.next = read_size_result.Move();

	// success
	return out;
}

//----------------------------------------------------------------------------->
// Writes parameter and all child nodes of this parameter.
//    @param node - reference to the node that is being serialized.
//    @param start - start position of the node in the binary stream.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteParameter(Snapshot& node, stream::Cursor start)
{
	// save parameter body
	Optional<Error> save_param_error = node.data.parameter->Save(*this);
	if (save_param_error)
	{
		return save_param_error;
	}

	// write children (this step is recursive)
	Optional<Error> save_children_error = WriteChildNodes(node, start);
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
//    @param start - start position of the node in the binary stream.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadParameter(Snapshot& node, stream::Cursor start)
{
	// here parameter loads it's value and/or attributes that
	// help it to build a new (deserialized) reflection tree, but with empty leaves
	Optional<Error> load_param_error = node.data.parameter->Load(*this);
	if (load_param_error)
	{
		return load_param_error;
	}

	// reflect once again loaded parameter to create those new 'empty leaves'
	node.Reset();
	node.data.parameter->Reflect(node);

	// after the tree structure was restored we can load every leaf separately
	Optional<Error> read_children_error = ReadChildNodes(node, start);
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
//    @param start - start position of the node in the binary stream.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteChildNodes(Snapshot& node, stream::Cursor start)
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
	const SizeType child_num = static_cast<SizeType>(node.CountChildren());
	Optional<Error> child_num_error = WriteNumberOfChildNodes(child_num);
	if (child_num_error)
	{
		return child_num_error;
	}

	// check case when parent node has the same stream offset as it's first
	// child node - then one needs to add an offset to the child node so that
	// it had different linkage id
	if (mode == Mode::binary_output && child_num != 0)
	{
		Result<stream::Cursor, Error> current_cursor = io.binary_output->GetCursor();
		if (!current_cursor)
		{
			return current_cursor.MoveAlt();
		}

		if (current_cursor.Get() == start)
		{
			ut::byte offset_stub = 0;
			Optional<Error> offset_error = WriteBinary<ut::byte>(&offset_stub, 1);
			if (offset_error)
			{
				return offset_error;
			}
		}
	}

	// allocate space for child nodes (only text mode is involved)
	Result<size_t, Error> alloc_result = AllocateChildNodes(node.CountChildren());
	if (!alloc_result)
	{
		return alloc_result.MoveAlt();
	}

	// save current state
	Controller state = SaveState();

	// iterate child nodes
	const size_t offset = alloc_result.Get();
	for (size_t i = 0; i < node.CountChildren(); i++)
	{
		// create a new node for serialiation
		Optional<Error> dive_new_node_error = DiveIntoChildNode(i + offset);
		if (dive_new_node_error)
		{
			return dive_new_node_error;
		}

		// write child node
		SerializationOptions options;
		options.initialize = false;
		Optional<Error> save_child_error = WriteNode(node[i], options);
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
//    @param start - start position of the node in the binary stream.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadChildNodes(Snapshot& node, stream::Cursor start)
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
	if (node.CountChildren() == 0)
	{
		return Optional<Error>();
	}

	// check case when parent node has the same stream offset as it's first
	// child node - then one needs to add an offset to the child node so that
	// it had different linkage id
	if (mode == Mode::binary_input)
	{
		Result<stream::Cursor, Error> current_cursor = io.binary_input->GetCursor();
		if (!current_cursor)
		{
			return current_cursor.MoveAlt();
		}

		if (current_cursor.Get() == start)
		{
			ut::byte offset_stub = 0;
			Optional<Error> offset_error = ReadBinary<ut::byte>(&offset_stub, 1);
			if (offset_error)
			{
				return offset_error;
			}
		}
	}

	// save current state
	Controller state = SaveState();

	// iterate child nodes
	for (size_t i = 0; i < child_num_result.Get(); i++)
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
		size_t node_id = i >= node.CountChildren() ? node.CountChildren() - 1 : i;

		// read child node
		SerializationOptions options;
		options.initialize = false;
		Result<Controller::Uniform, Error> read_result = ReadNode(node[node_id], options);
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
	if (mode == Mode::binary_output)
	{
		return info.HasBinaryNames() ? WriteBinary<String>(&name, 1) : Optional<Error>();
	}
	else if (mode == Mode::text_output)
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
	if (mode == Mode::binary_input)
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
	else if (mode == Mode::text_input)
	{
		return Optional<String>(io.text_input->data.name);
	}

	return MakeError(error::fail, "Invalid mode.");
}

//----------------------------------------------------------------------------->
// Writes a number of leaves in a node.
//    @param count - number of leaves to be written.
//    @return ut::Error if failed.
Optional<Error> Controller::WriteNumberOfChildNodes(size_t count)
{
	if (mode == Mode::binary_output)
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
	else if (mode == Mode::text_output)
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
	if (mode == Mode::binary_input)
	{
		// if there is no names - there is no number of children, we can only hope
		// that the number of serialized children matches current number of children
		if (!info.HasBinaryNames())
		{
			return node.CountChildren();
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
	else if (mode == Mode::text_input)
	{
		// text document retrieves a number of child nodes during a
		// parsing process, so this information is already known
		return io.text_input->CountChildren();
	}

	return MakeError(error::fail, "Invalid mode.");
}

//----------------------------------------------------------------------------->
// Allocates space for text child nodes (leaves) so that
// every child node had stable address.
//    @param count - number of child nodes.
//    @return - id of the first child, or error if something went wrong.
Result<size_t, Error> Controller::AllocateChildNodes(size_t count)
{
	size_t id = 0;
	if (mode == Mode::text_output)
	{
		// get id of the first leaf
		id = io.text_output->CountChildren();

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
Result<Optional<stream::Cursor>, Error> Controller::ReserveParameterSize()
{
	// this function is sensible only for a binary variant
	// and only if parameters can be skipped
	if (mode != Mode::binary_output || !SkipIsPossible())
	{
		return Optional<stream::Cursor>();
	}

	// write a number of bytes in the parameter, so that binary stream could skip it
	// if something would go wrong
	Result<stream::Cursor, Error> start_cursor_result = io.binary_output->GetCursor();
	if (!start_cursor_result)
	{
		error::Code error_code = start_cursor_result.GetAlt().GetCode();
		return MakeError(error_code, "Stream doesn't support positioning.");
	}

	// extract starting position
	const stream::Cursor position = start_cursor_result.Get();

	// write a size of the parameter
	SizeType parameter_size = 0;
	Optional<Error> save_size_error = WriteBinary<SizeType>(&parameter_size, 1);
	if (save_size_error)
	{
		return MakeError(save_size_error.Move());
	}

	// return a position of the size variable in stream
	return Optional<stream::Cursor>(position);
}

//----------------------------------------------------------------------------->
// Writes a size of a parameter to the binary stream.
//    @param start_position - position of the preallocated space for
//                            the size variable in a stream.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteParameterSize(const Optional<stream::Cursor>& start_position)
{
	// this function is sensible only for a binary variant
	// and only if parameters have names - so that they could be randomly iterated
	if (mode != Mode::binary_output || !start_position)
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
	const stream::Cursor end_position = end_position_result.Get();
	SizeType parameter_size = static_cast<SizeType>(end_position - start_position.Get());

	// move back cursor position
	Optional<Error> move_back_error = io.binary_output->MoveCursor(start_position.Get());
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
	if (mode != Mode::binary_input || !SkipIsPossible())
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
	return start_pos_result.Get() + static_cast<stream::Cursor>(parameter_size);
}

//----------------------------------------------------------------------------->
// Checks if name of the provided node matches provided name, and if not - searches
// for a sibling node with such a name.
//    @param node - reference to a node to check.
//    @param name - desired node name.
//    @return - a reference to the desired node, or ut::Error if failed.
Optional<Snapshot&> Controller::FindSiblingNode(Snapshot& node, const Optional<String>& name)
{
	// if there is no name - we can't look for the sibling node by name:
	// the only possible candidate is the current node
	if (!name)
	{
		return node;
	}

	// check name of the provided node
	if (node.data.name == name.Get())
	{
		return node;
	}

	// check siblings of the node
	Optional<Snapshot&> parent = node.GetParent();
	if (parent)
	{
		Optional<Snapshot&> find_result = parent->FindChildByName(name.Get());
		if (find_result)
		{
			return find_result;
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
	return Optional<Snapshot&>();
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
Optional<Error> Controller::SkipParameter(const Optional<stream::Cursor>& next)
{
	if (!next)
	{
		return Error(error::fail, "Parameters without size information cannot be skipped.");
	}

	// check if skip is possible
	if (!SkipIsPossible())
	{
		return Error(error::not_supported);
	}

	// this function is sensible only for binary variant
	if (mode != Mode::binary_input)
	{
		return Optional<Error>();
	}

	// skip this parameter
	return SetCursor(next.Get(), true);
}

//----------------------------------------------------------------------------->
// Returns 'true' if parameters can be skipped without error if something went wrong.
bool Controller::SkipIsPossible() const
{
	bool is_text_mode = mode == Mode::text_input || mode == Mode::text_output;

	// Skipping is sensible only if we can check type or name, in other words - only 
	// if we can detect that serialized parameter doesn't match a current one
	return is_text_mode || info.HasSizeinformation();
}

//----------------------------------------------------------------------------->
// Searches for a desired node inside current text node, and changes input/output
// source/target to this value node.
//    @param name - name of the node.
//    @return - ut::Error if failed.
Optional<Error> Controller::DiveIntoNamedNode(const String& name)
{
	// this function is sensible only for text variant
	if (mode == Mode::text_output)
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
	else if (mode == Mode::text_input)
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
	info.SetEndianness(endianness::Order::little);

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
	info.SetEndianness(endianness::Order::little);

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
	info.SetVersion(version.Get());
	info.SetFlags(flags.Get());

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
		if (shared_objects.Count() == 0)
		{
			break;
		}

		// write parameters
		for (size_t i = 0; i < shared_objects.Count(); i++)
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
	SetCursor(count_cursor.Get(), true);
	optional_error = WriteAttribute(count, node_names::skCount, true);
	if (optional_error)
	{
		return optional_error;
	}

	// set cursor back (where all shared parameters are already written)
	SetCursor(current_cursor.Get(), true);

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
	for (size_t i = 0; i < count.Get(); i++)
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
	Controller node_state = SaveState();

	// read information (name, type, id) and nothing else
	SerializationOptions options;
	options.initialize = false;
	options.only_uniforms = true;
	options.force_size_info = true;
	Result<Uniform, Error> uniform = ReadNode(Optional<Snapshot&>(), options);
	if (!uniform)
	{
		return uniform.MoveAlt();
	}

	// id must be present
	if (!uniform.Get().id)
	{
		String error_desc = "Shared object has no \"id\" value.";
		info.LogMessage(error_desc);
		return Error(error::fail, error_desc);
	}

	// iterate all registry entries to find id match
	for (size_t i = registry.Count(); i-- > 0;)
	{
		// check if id matches
		if (registry[i].id != static_cast<size_t>(uniform.Get().id.Get()))
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
		for (size_t j = 0; j < new_registry_entries.Count(); j++)
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
	SetCursor(stream_pos.Get());

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
	if (mode == Mode::text_output)
	{
		// get reference to the parent node
		Tree<text::Node>& parent = *io.text_output;

		// check range
		if (id >= parent.CountChildren())
		{
			return Error(error::out_of_bounds);
		}

		// set new output target
		io.text_output = &parent[id];
	}
	else if (mode == Mode::text_input)
	{
		// get reference to the parent node
		const Tree<text::Node>& parent = *io.text_input;

		// check range
		if (id >= parent.CountChildren())
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

//----------------------------------------------------------------------------->
// Modifies @info object according to the provided options.
// Note that one must restore original @info after reading/writing
// a node is done.
//    @param options - set of options.
//    @return - original (unmodified) copy of the @info object;
Info Controller::ModifyInfo(const SerializationOptions& options)
{
	Info original(info);

	if (options.force_size_info)
	{
		info.EnableSizeInformation(true);
	}

	return original;
}

// Reads custom data from the binary stream.
//    @address - pointer to the data to be read.
//    @granularity - size of one element.
//    @count - number of elements to write.
//    @return - ut::Error if failed.
Optional<Error> Controller::ReadBinary(void* address,
                                       size_t granularity,
                                       size_t count)
{
	// check mode
	if (mode != Mode::binary_input)
	{
		return Error(error::fail, "Invalid mode.");
	}

	// read elements using correct endianness order
	Optional<Error> read_error;
	if (info.GetEndianness() == endianness::Order::little)
	{
		read_error = endianness::Read<endianness::Order::little>(*io.binary_input,
		                                                         address,
		                                                         granularity,
		                                                         count);
	}
	else
	{
		read_error = endianness::Read<endianness::Order::big>(*io.binary_input,
		                                                      address,
		                                                      granularity,
		                                                      count);
	}

	// check read result
	if (read_error)
	{
		return read_error;
	}

	// update cursor position
	Result<stream::Cursor, Error> read_cursor = io.binary_input->GetCursor();
	if (!read_cursor)
	{
		return read_cursor.MoveAlt();
	}
	cursor = read_cursor.Get();

	// success
	return Optional<Error>();
}

// Writes custom data to the binary stream.
//    @address - pointer to the data to be written.
//    @granularity - size of one element.
//    @count - number of elements to write.
//    @return - ut::Error if failed.
Optional<Error> Controller::WriteBinary(const void* address,
                                        size_t granularity,
                                        size_t count)
{
	// check mode
	if (mode != Mode::binary_output)
	{
		return Error(error::fail, "Invalid mode.");
	}

	// write elements using correct endianness order
	Optional<Error> write_error;
	if (info.GetEndianness() == endianness::Order::little)
	{
		write_error = endianness::Write<endianness::Order::little>(*io.binary_output,
		                                                           address,
		                                                           granularity,
		                                                           count);
	}
	else
	{
		write_error = endianness::Write<endianness::Order::big>(*io.binary_output,
		                                                        address,
		                                                        granularity,
		                                                        count);
	}

	// check write result
	if (write_error)
	{
		return write_error;
	}

	// update cursor position
	Result<stream::Cursor, Error> read_cursor = io.binary_output->GetCursor();
	if (!read_cursor)
	{
		return read_cursor.MoveAlt();
	}
	cursor = read_cursor.Get();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//