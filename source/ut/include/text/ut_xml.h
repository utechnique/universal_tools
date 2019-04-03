//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "text/ut_document.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::XmlDoc is a class for XML Documents.
// Inspired by RapidXml (Marcin Kalicinski).
class XmlDoc : public text::Document
{
public:
	// Default constructor
	XmlDoc();

	// Copy constructor
	XmlDoc(const XmlDoc& copy);

	// Move constructor
#if CPP_STANDARD >= 2011
	XmlDoc(XmlDoc && rvalue);
#endif

	// Assignment operator
	XmlDoc& operator = (const XmlDoc& copy);

	// Move operator
#if CPP_STANDARD >= 2011
	XmlDoc& operator = (XmlDoc && rvalue);
#endif

	// Parses raw text
	//    @param text - string with a text to be parsed
	//    @return - ut::Error if encountered an error
	Optional<Error> Parse(const String& doc);

	// Writes contents to the output stream
	//    @param stream - output stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Write(OutputStream& stream) const;

private:
	// Parses BOM, if any
	//    @param cursor - reference to the current reader position
	void ParseBOM(text::Reader& cursor) const;
	
	// Determines node type first, then parses it
	//    @param cursor - reference to the current reader position
	//    @return - new node or ut::Error if encountered an error
	Result<Tree<text::Node>, Error> ParseNode(text::Reader& cursor);

	// Parses element node
	//    @param cursor - reference to the current reader position
	//    @return - new node or ut::Error if encountered an error
	Result<Tree<text::Node>, Error> ParseElement(text::Reader& cursor);

	// Parses XML declaration (<?xml...)
	//    @param cursor - reference to the current reader position
	//    @return - new node or ut::Error if encountered an error
	Result<Tree<text::Node>, Error> ParseXmlDeclaration(text::Reader& cursor);

	// Parses PI
	//    @param cursor - reference to the current reader position
	//    @return - new node or ut::Error if encountered an error
	Result<Tree<text::Node>, Error> ParsePi(text::Reader& cursor);

	// Parses XML comment (<!--...)
	//    @param cursor - reference to the current reader position
	//    @return - new node or ut::Error if encountered an error
	Result<Tree<text::Node>, Error> ParseComment(text::Reader& cursor);

	// Parses CDATA
	//    @param cursor - reference to the current reader position
	//    @return - new node or ut::Error if encountered an error
	Result<Tree<text::Node>, Error> ParseCData(text::Reader& cursor);

	// Parses DOCTYPE
	//    @param cursor - reference to the current reader position
	//    @return - new node or ut::Error if encountered an error
	Result<Tree<text::Node>, Error> ParseDoctype(text::Reader& cursor);

	// Parses XML attributes of the node
	//    @param cursor - reference to the current reader position
	//    @param node - reference to the parent node
	//    @return - ut::Error if encountered an error
	Optional<Error> ParseNodeAttributes(text::Reader& cursor, Tree<text::Node>& node);

	// Parses contents of the node - children, data etc.
	//    @param cursor - reference to the current reader position
	//    @param node - reference to the parent node
	//    @param validate_closing_tag - boolean if method must return error for
	//                                  invalid closing tg
	//    @return - ut::Error if encountered an error
	Optional<Error> ParseNodeContents(text::Reader& cursor,
	                                  Tree<text::Node>& node,
	                                  bool validate_closing_tag);

	// Parses and appends data
	// Returns character that ends data.
	// This is necessary because this character might have been overwritten by a terminating 0
	//    @param node - reference to the parent node
	//    @param cursor - reference to the current reader position
	//    @param contents_start - pointer to the first character of the data
	//    @param normalize_whitespace - boolean whether to normalize whitespaces or not
	//    @param trim_whitespace - boolean whether to trim whitespaces or not
	//    @return - character or ut::Error if encountered an error
	Result<char, Error> ParseAndAppendData(Tree<text::Node>& node,
	                                       text::Reader& cursor,
	                                       const char* contents_start,
	                                       bool normalize_whitespace,
	                                       bool trim_whitespace);

	// Skips characters until predicate evaluates to true while doing the following:
	// - replacing XML character entity references with proper characters (&apos; &amp; &quot; &lt; &gt; &#...;)
	// - condensing whitespace sequences to single space character
	//    @param cursor - reference to the current reader position
	//    @param stop_lookup_table - lookup table for skipping encountered characters
	//    @param stop_lookup_table_pure - lookup table for skipping encountered characters
	//    @param normalize_whitespace - boolean whether to normalize whitespaces or not
	//    @return - pointer to the next character or ut::Error if encountered an error
	static Result<String, Error> SkipAndExpandCharacterRefs(text::Reader& cursor,
	                                                        const byte* stop_lookup_table,
	                                                        const byte* stop_lookup_table_pure,
	                                                        bool normalize_whitespaces);

	// Writes specified node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteNode(OutputStream& stream,
	                                 const Tree<text::Node>& node,
	                                 uint32 depth = 0);

	// Writes general node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteGeneralNode(OutputStream& stream,
	                                        const Tree<text::Node>& node,
	                                        uint32 depth = 0);

	// Writes comment node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteComment(OutputStream& stream,
	                                    const Tree<text::Node>& node,
	                                    uint32 depth = 0);

	// Writes xml declaration node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteDeclaration(OutputStream& stream,
	                                        const Tree<text::Node>& node,
	                                        uint32 depth = 0);

	// Writes cdata node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteCData(OutputStream& stream,
	                                  const Tree<text::Node>& node,
	                                  uint32 depth = 0);

	// Writes doctype node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteDocType(OutputStream& stream,
	                                    const Tree<text::Node>& node,
	                                    uint32 depth = 0);

	// Writes process instruction node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @param depth - callstack recursive depth of the function
	//    @return - ut::Error if encountered an error
	static Optional<Error> WritePI(OutputStream& stream,
	                               const Tree<text::Node>& node,
	                               uint32 depth = 0);

	// Writes specified attribute node to the output stream
	//    @param stream - output stream
	//    @param node - node to be written
	//    @return - ut::Error if encountered an error
	static Optional<Error> WriteAttribute(OutputStream& stream, const Tree<text::Node>& node);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//