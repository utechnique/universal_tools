//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
#include "containers/ut_array.h"
#include "pointers/ut_unique_ptr.h"
#include "streams/ut_input_stream.h"
#include "streams/ut_output_stream.h"
#include "text/ut_document.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Archive is an abstract class providing input/output feautures.
// Save(ut::OutputStream&) and Load(InputStream&) methods for binary format
// must be implemented in the derived classes. Save(Tree<utext::Node>&) and
// Load(const Tree<text::Node>&) methods can be left "as is", but they will
// return 'error::not_implemented' code while trying to save/load text 
// (human-readable) data. So you have to overload these two methods too, if
// you intend to use archive object in a text mode.
class Archive
{
public:
	// Writes managed object data to the stream. Must be implemented by
	// derived class (child).
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(OutputStream& stream) = 0;

	// Loads managed object data from the stream. Must be implemented by
	// derived class (child).
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(InputStream& stream) = 0;

	// Writes managed object data to the text node. Returns ut::error::not_implemented
	// if it's not overriden by the child class. So you have to imlement it in the
	// derived class to be able to serialize parameter as a text data.
	//    @param node - text node to contain the managed data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(Tree<text::Node>& node);

	// Loads managed object data from the text node. Returns ut::error::not_implemented
	// if it's not overriden by the child class. So you have to imlement it in the
	// derived class to be able to serialize parameter as a text data.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(const Tree<text::Node>& node);

	// String with the name of ut::Archive type
	static const String skTypeName;
};

//----------------------------------------------------------------------------//
// Stream manipulator to save an archive as a text data using ut::text::Document
//    @param doc - text document (Xml, JSon, ..) to contain archive data
//    @param archive - archive to be saved in the document
//    @return - formatted text document
text::Document& operator << (text::Document& doc, Archive& archive);

//----------------------------------------------------------------------------//
// Stream manipulator to load an archive from text data using ut::text::Document
//    @param doc - text document (Xml, JSon, ..) to contain archive data
//    @param archive - archive to be saved in the document
//    @return - formatted text document
text::Document& operator >> (text::Document& doc, Archive& archive);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//