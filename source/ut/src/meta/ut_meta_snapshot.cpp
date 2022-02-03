//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_snapshot.h"
#include "meta/ut_meta_controller.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::skLogSerializationEvents static variable indicates if exceptional
// serialization events must be logged to the global log (ut::log).
static const bool skLogSerializationEvents = true;

//----------------------------------------------------------------------------//
// Constructor
//    @param info_copy - copy of the serialization info, that will be
//                       used during serialization and deserialization
Snapshot::Snapshot(Info info_copy) : Base(), info(MakeUnsafeShared<Info>(Move(info_copy)))
{ }

//----------------------------------------------------------------------------->
// Renames the snapshot.
void Snapshot::Rename(String new_name)
{
	data.name = Move(new_name);
}

//----------------------------------------------------------------------------->
// Saves full tree to a binary stream.
//    @param stream - reference to the output stream to serialize a tree to
//    @return - optionally ut::Error if failed
Optional<Error> Snapshot::Save(OutputStream& stream)
{
	// create a new controller using current information object
	Controller controller(info.GetRef());

	// change mode of the controller
	Optional<Error> mode_error = controller.SetBinaryOutputStream(stream);
	if (mode_error)
	{
		return mode_error;
	}

	// call pre-save callback functions
	InvokeCallback(&Snapshot::presave);

	// write self via controller
	meta::Controller::SerializationOptions default_options;
	Optional<Error> write_error = controller.WriteNode(*this, default_options);
	if (write_error)
	{
		return write_error;
	}

	// call post-save callback functions
	InvokeCallback(&Snapshot::postsave);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Loads full tree from a stream.
//    @param stream - reference to the input stream to deserialize from
//    @return - optionally ut::Error if failed
Optional<Error> Snapshot::Load(InputStream& stream)
{
	// create a new controller using current information object
	Controller controller(info.GetRef());

	// change mode of the controller
	Optional<Error> mode_error = controller.SetBinaryInputStream(stream);
	if (mode_error)
	{
		return mode_error;
	}
	
	// call pre-load callback functions
	InvokeCallback(&Snapshot::preload);

	// read self via controller
	meta::Controller::SerializationOptions default_options;
	Result<Controller::Uniform, Error> read_result = controller.ReadNode(*this, default_options);
	if (!read_result)
	{
		return read_result.MoveAlt();
	}

	// call post-load callback functions
	InvokeCallback(&Snapshot::postload);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Saves full tree to a text node.
//    @param text_node - reference to the text node to be serialized
//    @return - optionally ut::Error if failed
Optional<Error> Snapshot::Save(Tree<text::Node>& text_node)
{
	// create a new controller using current information object
	Controller controller(info.GetRef());

	// change mode of the controller
	Optional<Error> mode_error = controller.SetTextOutputNode(text_node);
	if (mode_error)
	{
		return mode_error;
	}

	// call pre-save callback functions
	InvokeCallback(&Snapshot::presave);

	// write self via controller
	meta::Controller::SerializationOptions default_options;
	Optional<Error> write_error = controller.WriteNode(*this, default_options);
	if (write_error)
	{
		return write_error;
	}

	// call pre-save callback functions
	InvokeCallback(&Snapshot::postsave);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Loads full tree from a text node.
//    @param text_node - reference to the text node to be deserialized
//    @return - optionally ut::Error if failed
Optional<Error> Snapshot::Load(const Tree<text::Node>& text_node)
{
	// create a new controller using current information object
	Controller controller(info.GetRef());

	// change mode of the controller
	Optional<Error> mode_error = controller.SetTextInputNode(text_node);
	if (mode_error)
	{
		return mode_error;
	}

	// call pre-load callback functions
	InvokeCallback(&Snapshot::preload);

	// read self via controller
	meta::Controller::SerializationOptions default_options;
	Result<Controller::Uniform, Error> read_result = controller.ReadNode(*this, default_options);
	if (!read_result)
	{
		return read_result.MoveAlt();
	}
	
	// call post-load callback functions
	InvokeCallback(&Snapshot::postload);

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Searches for a child node with a specific name.
//    @param node_name - name of the node to search for.
//    @return - reference to the node, or error if not found.
Optional<Snapshot&> Snapshot::FindChildByName(const String& node_name)
{
	// skip first slash (if present)
	const char* start = node_name.ToCStr();
	if (start[0] == '/' || start[0] == '\\')
	{
		start++;
	}

	// get top most leaf name
	const char* pstr = start;
	while (*pstr != '\0' && *pstr != '/' && *pstr != '\\')
	{
		pstr++;
	}
	const String leaf_name = String(start, pstr - start);

	// search a leaf node by name
	const bool is_final_node = *pstr == '\0';
	for (size_t i = 0; i < Base::GetNumChildren(); i++)
	{
		Snapshot& leaf = Base::child_nodes[i];
		if (leaf_name == leaf.data.name)
		{
			return is_final_node ? Base::child_nodes[i] : leaf.FindChildByName(++pstr);
		}
	}

	// not found
	return Optional<Snapshot&>();
}

//----------------------------------------------------------------------------->
// Assigns a callback that will be called right before saving.
//    @param callback - function to be called.
void Snapshot::SetPreSaveCallback(const Function<void()>& callback)
{
	presave = callback;
}

//----------------------------------------------------------------------------->
// Assigns a callback that will be called after saving is done.
//    @param callback - function to be called.
void Snapshot::SetPostSaveCallback(const Function<void()>& callback)
{
	postsave = callback;
}

//----------------------------------------------------------------------------->
// Assigns a callback that will be called right before loading.
//    @param callback - function to be called.
void Snapshot::SetPreLoadCallback(const Function<void()>& callback)
{
	preload = callback;
}

//----------------------------------------------------------------------------->
// Assigns a callback that will be called after loading is done.
//    @param callback - function to be called.
void Snapshot::SetPostLoadCallback(const Function<void()>& callback)
{
	postload = callback;
}

//----------------------------------------------------------------------------->
// Checks if provided parameter name is valid.
bool Snapshot::ValidateParameterName(const String& name)
{
	const size_t len = name.Length();

	// name must have at least one character
	if (len == 0)
	{
		return false;
	}

	// name must not have special symbols (/,\,*,:,&,?,<,>,{,},',")
	for (size_t i = 0; i < len; i++)
	{
		const char c = name[i];
		if (c == '\\' ||
		    c == '/' ||
		    c == '*' ||
		    c == ':' ||
		    c == '&' ||
		    c == '?' ||
		    c == '<' ||
		    c == '>' ||
		    c == '{' ||
		    c == '}' ||
		    c == '\'' ||
		    c == '\"')
		{
			return false;
		}
	}

	// first character must not be a number
	if (ChIsNumber(name.GetFirst()))
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------->
// Private constructor to create child nodes that share
// the same serialization info structure
//    @param info_ptr - reference to the shared pointer with the serialization
//                      info structure, that is shared among all tree branches
Snapshot::Snapshot(const InfoSharedPtr& info_ptr) : Base(), info(info_ptr)
{ }

//----------------------------------------------------------------------------->
// Recursively calls desired callback.
//    @param callback_ptr - pointer to the member representing
//                          serialization event callback
void Snapshot::InvokeCallback(Function<void()> Snapshot::* callback_ptr)
{
	// recursively iterate all child nodes
	const size_t child_count = child_nodes.GetNum();
	for (size_t i = 0; i < child_count; i++)
	{
		Function<void()>& child_callback = child_nodes[i].*callback_ptr;
		if (child_callback.IsValid())
		{
			child_callback();
		}
	}

	// only then call own callback
	Function<void()>& callback = this->*callback_ptr;
	if (callback.IsValid())
	{
		callback();
	}
}

//----------------------------------------------------------------------------//
// Stream manipulator to save an archive as a text data using ut::text::Document
//    @param doc - text document (Xml, JSon, ..) to contain archive data
//    @param archive - archive to be saved in the document
//    @return - formatted text document
text::Document& operator << (text::Document& doc, Snapshot& snapshot)
{
	// save parameter to the text node
	Tree<text::Node> node;
	node.data.name = "document";
	Optional<Error> save_error = snapshot.Save(node);
	if (save_error)
	{
		throw Error(save_error.Get());
	}

	// add the node to the text document
	doc << node;

	// success
	return doc;
}

//----------------------------------------------------------------------------//
// Stream manipulator to load an archive from text data using ut::text::Document
//    @param doc - text document (Xml, JSon, ..) to contain archive data
//    @param archive - archive to be saved in the document
//    @return - formatted text document
text::Document& operator >> (text::Document& doc, Snapshot& snapshot)
{
	// read node
	Tree<text::Node> node;
	doc >> node;

	// load the archive from the node
	Optional<Error> load_error = snapshot.Load(node);
	if (load_error)
	{
		throw Error(load_error.Get());
	}

	// success
	return doc;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//