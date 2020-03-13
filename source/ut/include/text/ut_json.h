//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "text/ut_document.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//

class JsonDoc : public text::Document
{
public:
	// Parses raw text
	//    @param text - string with a text to be parsed
	//    @return - ut::Error if encountered an error
	Optional<Error> Parse(const String& doc);

	// Writes contents to the output stream
	//    @param stream - output stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Write(OutputStream& stream) const;

private:
	// Parses the value of the node
	//    @param cursor - reference to the current parsing position
	//    @param node - reference to the parent node
	//    @return - ut::Error if encountered an error
	static Optional<Error> ParseValue(text::Reader& cursor, Tree<text::Node>& node);

	// Parses '{}' object node
	//    @param cursor - reference to the current parsing position
	//    @param node - reference to the parent node
	//    @return - ut::Error if encountered an error
	static Optional<Error> ParseObject(text::Reader& cursor, Tree<text::Node>& node);

	// Parses '[]' array node
	//    @param cursor - reference to the current parsing position
	//    @param node - reference to the parent node
	//    @return - ut::Error if encountered an error
	static Optional<Error> ParseArray(text::Reader& cursor, Tree<text::Node>& node);

	// Extracts a JSON String as defined by the spec - "<some chars>"
	// Any escaped characters are swapped out for their unescaped values
	//    @param cursor - reference to the current parsing position
	//    @return - extracted string or ut::Error if encountered an error
	static Result<String, Error> ExtractString(text::Reader& cursor);

	// Extracts a JSON number
	//    @param cursor - reference to the current parsing position
	//    @return - extracted string or ut::Error if encountered an error
	static Result<String, Error> ExtractNumber(text::Reader& cursor);

	// Writes specified node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteNode(OutputStream& stream,
	                                 const Tree<text::Node>& node,
	                                 bool write_name,
	                                 uint32 depth = 0);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//