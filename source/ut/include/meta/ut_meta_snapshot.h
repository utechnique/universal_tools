//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_tree.h"
#include "pointers/ut_shared_ptr.h"
#include "text/ut_document.h"
#include "meta/ut_meta.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Snapshot is a class representing a reflective tree that is a
// reflective image of another object, or group of objects.
// Snapshot can be used for serializing/deserializing an object, see methods
// ut::meta::Snapshot::Save() and ut::meta::Snapshot::Load().
class Snapshot : public BaseTree<Node, Snapshot>
{
	typedef BaseTree<Node, Snapshot> Base;
	typedef SharedPtr<Info, ut::thread_safety::off> InfoSharedPtr;
public:
	
	// ut::meta::Snapshot::Capture static function is the only way to create a snapshot.
	// You must provide an object you want to reflect.
	//    @param object - reference to the object to be reflected.
	//    @param name - name of the parameter associated with the @object.
	//    @param info - copy of the serialization info, that will be
	//                  used during serialization and deserialization
	//    @return - ut::Snapshot object.
	template <typename T>
	static Snapshot Capture(T& object,
	                        const String& name = "snapshot",
	                        const Info& info = Info::CreateComplete())
	{
		Snapshot snapshot(info);
		Optional<Error> init_error = snapshot.Init<T>(object, name);
		if (init_error)
		{
			throw init_error;
		}
		return snapshot;
	}

	// Registers object that is provided as an argument,
	// parameter name is auto-generated
	//    @param ref - reference to the object to be serialized
	template<typename T>
	Snapshot& operator << (T& ref)
	{
		// auto-generated name
		String name;
		name.Print("p%u", static_cast<uint32>(Base::child_nodes.GetNum()));

		// add new parameter
		Optional<Error> add_param_error = Add(ref, name);
		if (add_param_error)
		{
			throw add_param_error.Move();
		}

		// success
		return *this;
	}

	// Registers object that is provided as an argument
	//    @param ref - reference to the object to be serialized
	//    @param name - name of the parameter
	template<typename T>
	Optional<Error> Add(T& ref, const String& name)
	{
		// create a new node
		Snapshot snapshot(info);

		// init child node
		Optional<Error> init_error = snapshot.Init<T>(ref, name);
		if (init_error)
		{
			return init_error;
		}

		// add node
		if (!Base::Add(Move(snapshot)))
		{
			return Error(error::out_of_memory);
		}

		// success
		return Optional<Error>();
	}

	// Saves full tree to a binary stream.
	//    @param stream - reference to the output stream to serialize a tree to
	//    @return - optionally ut::Error if failed
	Optional<Error> Save(OutputStream& stream);

	// Loads full tree from a binary stream.
	//    @param stream - reference to the input stream to deserialize from
	//    @return - optionally ut::Error if failed
	Optional<Error> Load(InputStream& stream);
	
	// Saves full tree to a text node.
	//    @param text_node - reference to the text node to be serialized
	//    @return - optionally ut::Error if failed
	Optional<Error> Save(Tree<text::Node>& text_node);
	
	// Loads full tree from a text node.
	//    @param text_node - reference to the text node to be deserialized
	//    @return - optionally ut::Error if failed
	Optional<Error> Load(const Tree<text::Node>& text_node);

	// Searches for a child node with a specific name.
	//    @param node_name - name of the node to search for.
	//    @return - reference to the node, or error if not found.
	Optional< Ref<Snapshot> > FindChildByName(const String& node_name);

private:
	// Private constructor
	//    @param info_copy - copy of the serialization info, that will be
	//                       used during serialization and deserialization
	Snapshot(const Info& info_copy = Info::CreateComplete());

	// Private constructor to create child nodes that share
	// the same serialization info structure
	//    @param info_ptr - reference to the shared pointer with the serialization
	//                      info structure, that is shared among all tree branches
	Snapshot(const InfoSharedPtr& info_ptr);

	// Builds a full reflection tree from a provided object.
	//    @param object - reference to the object to be reflected.
	//    @param name - name of the @object.
	//    @return - ut::Error if failed.
	template <typename T>
	Optional<Error> Init(T& object, const String& name)
	{
		// create one of the possible parameters - correct
		// variant is deduced from the argument list
		data.parameter = new Parameter<T>(&object);

		// copy parameter name
		data.name = name;

		// generate id
		data.id = 0;

		// register child nodes
		data.parameter->Reflect(*this);

		// success
		return Optional<Error>();
	}

	// Serialization info contains information how to 
	// serialize/deserialize managed object.
	InfoSharedPtr info;
};

//----------------------------------------------------------------------------//
// Stream manipulator to save an archive as a text data using ut::text::Document
//    @param doc - text document (Xml, JSon, ..) to contain archive data
//    @param archive - archive to be saved in the document
//    @return - formatted text document
text::Document& operator << (text::Document& doc, Snapshot& snapshot);

//----------------------------------------------------------------------------//
// Stream manipulator to load an archive from text data using ut::text::Document
//    @param doc - text document (Xml, JSon, ..) to contain archive data
//    @param archive - archive to be saved in the document
//    @return - formatted text document
text::Document& operator >> (text::Document& doc, Snapshot& snapshot);

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//