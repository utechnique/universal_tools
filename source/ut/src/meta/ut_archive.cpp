//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_archive.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// String with the name of ut::Archive type
const String Archive::skTypeName = "archive";

// Writes managed object data to the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node to contain the managed data
//    @return - ut::Error if encountered an error
Optional<Error> Archive::Save(Tree<text::Node>& node)
{
	return Error(error::not_implemented);
}

// Loads managed object data from the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node containing the managed data
//    @return - ut::Error if encountered an error
Optional<Error> Archive::Load(const Tree<text::Node>& node)
{
	return Error(error::not_implemented);
}

//----------------------------------------------------------------------------//
// Stream manipulator to save archive as a text data using ut::text::Document
//    @param doc - text document (Xml, JSon, ..) to contain archive data
//    @param archive - archive to be saved in @format document
//    @return - formatted text document
text::Document& operator << (text::Document& doc, Archive& archive)
{
	// save parameter to the text node
	Tree<text::Node> node;
	node.data.name = Archive::skTypeName;
	Optional<Error> save_error = archive.Save(node);
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
text::Document& operator >> (text::Document& doc, Archive& archive)
{
	// read node
	Tree<text::Node> node;
	doc >> node;

	// load the archive from the node
	Optional<Error> load_error = archive.Load(node);
	if (load_error)
	{
		throw Error(load_error.Get());
	}

	// success
	return doc;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//