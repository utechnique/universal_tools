//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_tree.h"
#include "error/ut_error.h"
#include "pointers/ut_unique_ptr.h"
#include "text/ut_string.h"
#include "text/ut_text_node.h"
#include "text/ut_text_reader.h"
#include "streams/ut_input_stream.h"
#include "streams/ut_output_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(text)
//----------------------------------------------------------------------------//
// ut::text::Document is a parent abstract class for the documents of different
// formats implementing Tree data structure. These could be Xml, Json, etc.
class Document
{
public:
	// Default constructor
	Document();

	// Virtual destructor.
	virtual ~Document() = default;

	// Parses raw text
	//    @param doc - string with a text to be parsed
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Parse(const String& doc) = 0;

	// Writes contents to the output stream
	//    @param stream - output stream
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Write(OutputStream& stream) const = 0;
	
	// Parses text file
	//    @param filename - string with a path to the file
	//    @return - ut::Error if encountered an error
	Optional<Error> ParseFile(const String& filename);

	// This operator adds a new node to the document
	Document& operator << (const Tree<Node>& node);

	// This operator moves a new node to the document
	Document& operator << (Tree<Node>&& node);

	// This operator reads a node from the document
	Document& operator >> (Tree<Node>& node);

	// text nodes
	Array< Tree<Node> > nodes;

	// some document types don't support simultaneous existing of
	// the child nodes and values (JSON for example), to save this type
	// of documents we need to wrap the value into another child node, and
	// this node must have the following name - ut::text::Document::skValueNodeName
	static const char* skValueNodeName;

protected:
	// Skips characters until predicate evaluates to true
	//    @param txt - reference to the pointer to the current character
	//    @param lookup_table - lookup table for skipping encountered characters
	static void Skip(Reader& txt, const byte* lookup_table);

	// Generates a string with a specified number of tabs
	//    @param num - number of tabs
	//    @return - string with the specified number of tabs
	static String GenerateTabs(uint32 num);

	// Lookup tables
	struct Lookup
	{
		// Whitespace (space \n \r \t)
		static const byte skWhitespace[256];

		// Node name (anything but space \n \r \t / > ? \0)
		static const byte skNodeName[256];

		// Attribute name (anything but space \n \r \t / < > = ? ! \0)
		static const byte skAttributeName[256];

		// Attribute data with single quote (anything but ' \0)
		static const byte skAttributeDataSingleQuotes[256];

		// Attribute data with single quote that does not require processing (anything but ' \0 &)
		static const byte skAttributeDataSingleQuotesPure[256];

		// Attribute data with double quote (anything but " \0)
		static const byte skAttributeDataDoubleQuotes[256];

		// Attribute data with double quote that does not require processing (anything but " \0 &)
		static const byte skAttributeDataDoubleQuotesPure[256];

		// Digits (dec and hex, 255 denotes end of numeric character reference)
		static const byte skDigits[256];

		// Text (i.e. PCDATA) (anything but < \0)
		static const byte skText[256];

		// Text (i.e. PCDATA) that does not require processing when ws normalization is disabled 
		// (anything but < \0 &)
		static const byte skTextPure[256];

		// Text (i.e. PCDATA) that does not require processing when ws normalizationis is enabled
		// (anything but < \0& space \n \r \t)
		static const byte skTextPureNoWhitespaces[256];
	};

private:
	// cursor for stream-like << and >> operators
	stream::Cursor cursor;
};

//----------------------------------------------------------------------------//
// Stream manipulator to write a document to the output stream
//    @param doc - text document (Xml, JSon, ..) to be written
//    @param stream - output stream
//    @return - formatted text document
Document& operator << (OutputStream& stream, Document& doc);

//----------------------------------------------------------------------------//
// Stream manipulator to read a document from the input stream
//    @param doc - text document (Xml, JSon, ..) to be written
//    @param stream - output stream
//    @return - formatted text document
Document& operator >> (InputStream& stream, Document& doc);

//----------------------------------------------------------------------------//
END_NAMESPACE(text)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//