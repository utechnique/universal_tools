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
Snapshot::Snapshot(Info info_copy) : Base(), info(new Info(Move(info_copy)))
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

	// write self via controller
	return controller.WriteNode(*this);
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

	// read self via controller
	Result<Controller::Uniform, Error> read_result = controller.ReadNode(Ref<Snapshot>(*this));
	if (!read_result)
	{
		return read_result.MoveAlt();
	}

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

	// write self via controller
	return controller.WriteNode(*this);
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

	// read self via controller
	Result<Controller::Uniform, Error> read_result = controller.ReadNode(Ref<Snapshot>(*this));
	if (!read_result)
	{
		return read_result.MoveAlt();
	}
	
	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Searches for a child node with a specific name.
//    @param node_name - name of the node to search for.
//    @return - reference to the node, or error if not found.
Optional< Ref<Snapshot> > Snapshot::FindChildByName(const String& node_name)
{
	for (size_t i = 0; i < Base::GetNumChildren(); i++)
	{
		if (node_name == Base::child_nodes[i].data.name)
		{
			return Ref<Snapshot>(Base::child_nodes[i]);
		}
	}
	return Optional< Ref<Snapshot> >();
}

//----------------------------------------------------------------------------->
// Private constructor to create child nodes that share
// the same serialization info structure
//    @param info_ptr - reference to the shared pointer with the serialization
//                      info structure, that is shared among all tree branches
Snapshot::Snapshot(const InfoSharedPtr& info_ptr) : Base(), info(info_ptr)
{ }

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