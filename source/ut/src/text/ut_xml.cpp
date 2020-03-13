//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "text/ut_xml.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Parses raw text
//    @param text - string with a text to be parsed
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::Parse(const String& doc)
{
	// create input object with the address of the provided string
	text::Reader cursor(doc.GetAddress());

	// remove current contents
	nodes.Empty();
	
	// parse BOM, if any
	ParseBOM(cursor);
	
	// parse children
	while (true)
	{
		// skip whitespace before node
		Skip(cursor, Lookup::skWhitespace);
		if (cursor == 0)
		{
			break;
		}

		// parse and append new child
		if (cursor == '<')
		{
			cursor++; // skip '<'
			Result<Tree<text::Node>, Error> parse_node_result = ParseNode(cursor);
			if (parse_node_result)
			{
				if (!nodes.Add(parse_node_result.MoveResult()))
				{
					return Error(error::out_of_memory);
				}
			}
		}
		else
		{
			return Error(error::fail, "expected <");
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes contents to the output stream
//    @param stream - output stream
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::Write(OutputStream& stream) const
{
	// write every node
	for (size_t i = 0; i < nodes.GetNum(); i++)
	{
		Optional<Error> save_node_error = WriteNode(stream, nodes[i]);
		if (save_node_error)
		{
			return save_node_error;
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Parses BOM, if any
//    @param cursor - reference to the current char pointer
void XmlDoc::ParseBOM(text::Reader& cursor) const
{
	if (cursor[0] != 0 && cursor[1] != 0 && cursor[2] != 0)
	{
		// UTF-8?
		if (static_cast<unsigned char>(cursor[0]) == 0xEF &&
			static_cast<unsigned char>(cursor[1]) == 0xBB &&
			static_cast<unsigned char>(cursor[2]) == 0xBF)
		{
			cursor += 3;      // Skup utf-8 bom
		}
	}
}

//----------------------------------------------------------------------------->
// Determines node type first, then parses it
//    @param cursor - reference ot the pointer to the current character
//    @return - new node or ut::Error if encountered an error
Result<Tree<text::Node>, Error> XmlDoc::ParseNode(text::Reader& cursor)
{
	// Parse proper node type
	switch (cursor[0])
	{
		// <...
		default:
		{
			// Parse and append element node
			return ParseElement(cursor);
		}

		// <?...
		case '?':
		{
			++cursor;     // Skip ?
			if ((cursor[0] == 'x' || cursor[0] == 'X') &&
				(cursor[1] == 'm' || cursor[1] == 'M') &&
				(cursor[2] == 'l' || cursor[2] == 'L') &&
				Lookup::skWhitespace[cursor[3]])
			{
				// '<?xml ' - xml declaration
				cursor += 4;      // Skip 'xml '
				return ParseXmlDeclaration(cursor);
			}
			else
			{
				// Parse PI
				return ParsePi(cursor);
			}
		}
		// <!...
		case '!':
		{
			// Parse proper subset of <! node
			switch (cursor[1])
			{

				// <!-
				case '-':
				{
					if (cursor[2] == '-')
					{
						// '<!--' - xml comment
						cursor += 3;     // Skip '!--'
						return ParseComment(cursor);
					}
				} break;

				// <![
				case '[':
				{
					if (cursor[2] == 'C' && cursor[3] == 'D' && cursor[4] == 'A' &&
						cursor[5] == 'T' && cursor[6] == 'A' && cursor[7] == '[')
					{
						// '<![CDATA[' - cdata
						cursor += 8;     // Skip '![CDATA['
						return ParseCData(cursor);
					}
				} break;

				// <!D
				case 'D':
				{
					if (cursor[2] == 'O' && cursor[3] == 'C' && cursor[4] == 'T' &&
						cursor[5] == 'Y' && cursor[6] == 'P' && cursor[7] == 'E' &&
						Lookup::skWhitespace[cursor[8]])
					{
						// '<!DOCTYPE ' - doctype
						cursor += 9;      // skip '!DOCTYPE '
						return ParseDoctype(cursor);
					}
				}
			} // switch cursor[1]

			// Attempt to skip other, unrecognized node types starting with <!
			++cursor;     // Skip !
			while (cursor != '>')
			{
				if (cursor == 0)
				{
					return MakeError(Error(error::fail, "Unexpected end of data"));
				}
				++cursor;
			}
			++cursor;     // Skip '>'

			return MakeError(Error(error::fail, "No node recognized"));
		} // case '!'
	} // switch (cursor[0])
}

//----------------------------------------------------------------------------->
// Parses element node
//    @param cursor - reference ot the pointer to the current character
//    @return - new node or ut::Error if encountered an error
Result<Tree<text::Node>, Error> XmlDoc::ParseElement(text::Reader& cursor)
{
	// Create element node
	Tree<text::Node> element;

	// Extract element name
	const char* name = cursor.Get();
	Skip(cursor, Lookup::skNodeName);
	if (cursor.Get() == name)
	{
		return MakeError(Error(error::fail, "expected element name"));
	}
	element.data.name = String(name, cursor.Get() - name);

	// Skip whitespace between element name and attributes or >
	Skip(cursor, Lookup::skWhitespace);

	// Parse attributes, if any
	Optional<Error> parse_attributes_error = ParseNodeAttributes(cursor, element);
	if (parse_attributes_error)
	{
		return MakeError(parse_attributes_error.Get());
	}

	// Determine ending type
	if (cursor == '>')
	{
		++cursor;
		Optional<Error> parse_contents_error = ParseNodeContents(cursor, element, true);
		if (parse_contents_error)
		{
			return MakeError(parse_contents_error.Get());
		}
	}
	else if (cursor == '/')
	{
		++cursor;
		if (cursor != '>')
		{
			return MakeError(Error(error::fail, "expected >"));
		}
		++cursor;
	}
	else
	{
		return MakeError(Error(error::fail, "expected >"));
	}

	// Return parsed element
	return element;
}

//----------------------------------------------------------------------------->
// Parses XML declaration (<?xml...)
//    @param cursor - reference ot the pointer to the current character
//    @return - new node or ut::Error if encountered an error
Result<Tree<text::Node>, Error> XmlDoc::ParseXmlDeclaration(text::Reader& cursor)
{
	// Create declaration
	Tree<text::Node> declaration(text::node::xml_declaration);

	// Skip whitespace before attributes or ?>
	Skip(cursor, Lookup::skWhitespace);

	// Parse declaration attributes
	Optional<Error> parse_attributes_error = ParseNodeAttributes(cursor, declaration);
	if (parse_attributes_error)
	{
		return MakeError(parse_attributes_error.Get());
	}

	// Skip ?>
	if (cursor[0] != '?' || cursor[1] != '>')
	{
		return MakeError(Error(error::fail, "expected ?>"));
	}
	cursor += 2;

	return declaration;
}

//----------------------------------------------------------------------------->
// Parses PI
//    @param cursor - reference ot the pointer to the current character
//    @return - new node or ut::Error if encountered an error
Result<Tree<text::Node>, Error> XmlDoc::ParsePi(text::Reader& cursor)
{
	// Create pi node
	Tree<text::Node> pi(text::node::xml_pi);

	// Extract PI target name
	const char* name = cursor.Get();
	Skip(cursor, Lookup::skNodeName);
	if (cursor.Get() == name)
	{
		return MakeError(Error(error::fail, "expected PI target"));
	}
	pi.data.name = String(name, cursor.Get() - name);

	// Skip whitespace between pi target and pi
	Skip(cursor, Lookup::skWhitespace);

	// Remember start of pi
	const char* value = cursor.Get();

	// Skip to '?>'
	while (cursor[0] != '?' || cursor[1] != '>')
	{
		if (cursor == '\0')
		{
			return MakeError(Error(error::fail, "unexpected end of data"));
		}
		++cursor;
	}

	// Set pi value (verbatim, no entity expansion or whitespace normalization)
	pi.data.value = String(value, cursor.Get() - value);
	
	// Skip '?>'
	cursor += 2;

	// success
	return pi;
}

//----------------------------------------------------------------------------->
// Parses XML comment (<!--...)
//    @param cursor - reference ot the pointer to the current character
//    @return - new node or ut::Error if encountered an error
Result<Tree<text::Node>, Error> XmlDoc::ParseComment(text::Reader& cursor)
{
	// Remember value start
	const char* value = cursor.Get();

	// Skip until end of comment
	while (cursor[0] != '-' || cursor[1] != '-' || cursor[2] != '>')
	{
		if (!cursor[0])
		{
			return MakeError(Error(error::fail, "unexpected end of data"));
		}
		++cursor;
	}

	// Create comment node
	Tree<text::Node> comment(text::node::comment);
	comment.data.value = String(value, cursor.Get() - value);

	// Skip '-->'
	cursor += 3;

	// success
	return comment;
}

//----------------------------------------------------------------------------->
// Parses CDATA
//    @param cursor - reference ot the pointer to the current character
//    @return - new node or ut::Error if encountered an error
Result<Tree<text::Node>, Error> XmlDoc::ParseCData(text::Reader& cursor)
{
	// Skip until end of cdata
	const char* value = cursor.Get();
	while (cursor[0] != ']' || cursor[1] != ']' || cursor[2] != '>')
	{
		if (!cursor[0])
		{
			return MakeError(Error(error::fail, "unexpected end of data"));
		}
		++cursor;
	}

	// Create new cdata node
	Tree<text::Node> cdata(text::node::xml_cdata);
	cdata.data.value = String(value, cursor.Get() - value);

	// Skip ]]>
	cursor += 3;

	// success
	return cdata;
}

//----------------------------------------------------------------------------->
// Parses DOCTYPE
//    @param cursor - reference ot the pointer to the current character
//    @return - new node or ut::Error if encountered an error
Result<Tree<text::Node>, Error> XmlDoc::ParseDoctype(text::Reader& cursor)
{
	// Remember value start
	const char* value = cursor.Get();

	// Skip to >
	while (cursor != '>')
	{
		// Determine character type
		switch (cursor[0])
		{

			// If '[' encountered, scan for matching ending ']' using naive algorithm with depth
			// This works for all W3C test files except for 2 most wicked
			case '[':
			{
				++cursor;     // Skip '['
				int depth = 1;
				while (depth > 0)
				{
					switch (cursor[0])
					{
						case '[': ++depth; break;
						case ']': --depth; break;
						case 0:
						{
							return MakeError(Error(error::fail, "unexpected end of data"));
						}
					}
					++cursor;
				}
				break;
			}

			// Error on end of text
			case '\0':
			{
				return MakeError(Error(error::fail, "unexpected end of data"));
			}

			// Other character, skip it
			default:
			{
				++cursor;
			}
		}
	}

	// Create a new doctype node
	Tree<text::Node> doctype(text::node::xml_doctype);
	doctype.data.value = String(value, cursor.Get() - value);

	// skip '>'
	cursor += 1;

	// success
	return doctype;
}

//----------------------------------------------------------------------------->
// Parses XML attributes of the node
//    @param cursor - reference to the current reader position
//    @param node - reference to the parent node
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::ParseNodeAttributes(text::Reader& cursor, Tree<text::Node>& node)
{
	// For all attributes 
	while (Lookup::skAttributeName[cursor[0]])
	{
		// Extract attribute name
		const char* name = cursor.Get();
		++cursor;     // Skip first character of attribute name
		Skip(cursor, Lookup::skAttributeName);
		if (cursor.Get() == name)
		{
			return Error(error::fail, "expected attribute name");
		}

		// Create new attribute
		Tree<text::Node> attribute;
		attribute.data.is_attribute = true;
		attribute.data.name = String(name, cursor.Get() - name);

		// Skip whitespace after attribute name
		Skip(cursor, Lookup::skWhitespace);

		// Skip =
		if (cursor != '=')
		{
			return Error(error::fail, "expected =");
		}
		++cursor;

		// Skip whitespace after =
		Skip(cursor, Lookup::skWhitespace);

		// Skip quote and remember if it was ' or "
		char quote = cursor[0];
		if (quote != '\'' && quote != '"')
		{
			return Error(error::fail, "expected ' or \"");
		}
		++cursor;

		// Extract attribute value and expand char refs in it
		const byte* lookup_table = Lookup::skAttributeDataSingleQuotes;
		const byte* lookup_table_pure = Lookup::skAttributeDataSingleQuotesPure;
		if (quote == '\"')
		{
			lookup_table = Lookup::skAttributeDataDoubleQuotes;
			lookup_table_pure = Lookup::skAttributeDataDoubleQuotesPure;
		}

		// skip and expand character ref
		Result<String, Error> result = SkipAndExpandCharacterRefs(cursor,
			                                                      lookup_table,
			                                                      lookup_table_pure,
			                                                      false);
		// Set attribute value
		if (result)
		{
			attribute.data.value = result.MoveResult();
		}
		else
		{
			return result.GetAlt();
		}

		// Make sure that end quote is present
		if (cursor != quote)
		{
			return Error(error::fail, "expected ' or \"");
		}
		++cursor; // Skip quote

		// Skip whitespace after attribute value
		Skip(cursor, Lookup::skWhitespace);

		// add attribute to the parent node
		if (!node.Add(Move(attribute)))
		{
			return Error(error::out_of_memory);
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Parses contents of the node - children, data etc.
//    @param cursor - reference to the current reader position
//    @param node - reference to the parent node
//    @param validate_closing_tag - boolean if method must return error for
//                                  invalid closing tg
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::ParseNodeContents(text::Reader& cursor,
                                          Tree<text::Node>& node,
                                          bool validate_closing_tag)
{
	// For all children and text
	while (1)
	{
		// Skip whitespace between > and node contents
		const char* contents_start = cursor.Get(); // Store start of node contents before whitespace is skipped
		Skip(cursor, Lookup::skWhitespace);
		char next_char = cursor[0];

		// After data nodes, instead of continuing the loop, control jumps here.
		// This is because zero termination inside parse_and_append_data() function
		// would wreak havoc with the above code.
		// Also, skipping whitespace after data nodes is unnecessary.
	after_data_node:

		// Determine what comes next: node closing, child node, data node, or 0?
		switch (next_char)
		{
			// Node closing or child node
			case '<':
			{
				if (cursor[1] == '/')
				{
					// Node closing
					cursor += 2;      // Skip '</'
					if (validate_closing_tag)
					{
						// Skip and validate closing tag name
						const char* closing_name_start = cursor.Get();
						Skip(cursor, Lookup::skNodeName);
						String closing_name(closing_name_start, cursor.Get() - closing_name_start);
						if (closing_name != node.data.name)
						{
							return Error(error::fail, "invalid closing tag name");
						}
					}
					else
					{
						// No validation, just skip name
						Skip(cursor, Lookup::skNodeName);
					}

					// Skip remaining whitespace after node name
					Skip(cursor, Lookup::skWhitespace);
					if (cursor != '>')
					{
						return Error(error::fail, "expected >");
					}

					// Skip '>'
					++cursor;

					// Node closed, finished parsing contents
					return Optional<Error>();
				}
				else
				{
					// Child node
					++cursor; // Skip '<'

					Result<Tree<text::Node>, Error> parse_node_result = ParseNode(cursor);
					if (parse_node_result)
					{
						node.Add(parse_node_result.MoveResult());
					}
					else
					{
						return parse_node_result.GetAlt();
					}
				}
			} break;

			// End of data - error
			case '\0':
			{
				return Error(error::fail, "unexpected end of data");
			}

			// Data node
			default:
			{
				Result<char, Error> result = ParseAndAppendData(node,
				                                                cursor,
				                                                contents_start,
				                                                true,
				                                                true);

				if (result)
				{
					next_char = result.GetResult();
					goto after_data_node;   // Bypass regular processing after data nodes
				}
				else
				{
					return result.GetAlt();
				}
			}
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Parses and appends data
// Returns character that ends data.
// This is necessary because this character might have been overwritten by a terminating 0
//    @param node - reference to the parent node
//    @param cursor - reference to the current reader position
//    @param contents_start - pointer to the first character of the data
//    @param normalize_whitespace - boolean whether to normalize whitespaces or not
//    @param trim_whitespace - boolean whether to trim whitespaces or not
//    @return - ut::Error if encountered an error
Result<char, Error> XmlDoc::ParseAndAppendData(Tree<text::Node>& node,
                                               text::Reader& cursor,
                                               const char* contents_start,
                                               bool normalize_whitespace,
                                               bool trim_whitespace)
{
	// Backup to contents start if whitespace trimming is disabled
	if (!trim_whitespace)
	{
		cursor = contents_start;
	}

	// Skip until end of data
	const byte* lookup_table_pure = normalize_whitespace ? Lookup::skTextPure : Lookup::skTextPureNoWhitespaces;
	Result<String, Error> result = SkipAndExpandCharacterRefs(cursor,
	                                                          Lookup::skText,
	                                                          lookup_table_pure,
	                                                          normalize_whitespace);
	// set value
	if (result)
	{
		// move value string
		String value(result.MoveResult());
		size_t val_length = value.Length();

		// Trim trailing whitespace if flag is set; leading was already trimmed by whitespace skip after >
		if (trim_whitespace && val_length > 0)
		{
			// Backup until non-whitespace character is found
			while (Lookup::skWhitespace[value[val_length - 1]] && val_length > 0)
			{
				value.Remove(val_length - 1);
				--val_length;
			}
		}

		// Add data to parent node if no data exists yet
		node.data.value = Move(value);
	}
	else
	{
		return MakeError(result.GetAlt());
	}

	// Return character that ends data
	return cursor[0];
}

//----------------------------------------------------------------------------->
// Skips characters until predicate evaluates to true while doing the following:
// - replacing XML character entity references with proper characters (&apos; &amp; &quot; &lt; &gt; &#...;)
// - condensing whitespace sequences to single space character
//    @param cursor - reference to the current reader position
//    @param stop_lookup_table - lookup table for skipping encountered characters
//    @param stop_lookup_table_pure - lookup table for skipping encountered characters
//    @param normalize_whitespace - boolean whether to normalize whitespaces or not
//    @return - pointer to the next character or ut::Error if encountered an error
Result<String, Error> XmlDoc::SkipAndExpandCharacterRefs(text::Reader& cursor,
                                                         const byte* stop_lookup_table,
                                                         const byte* stop_lookup_table_pure,
                                                         bool normalize_whitespaces)
{
	// Use simple skip until first modification is detected
	const char * start = cursor.Get();
	Skip(cursor, stop_lookup_table_pure);

	// Use translation skip
	String out(start, cursor.Get() - start);
	while (stop_lookup_table[cursor[0]])
	{
		// If entity translation is enabled    
		if (true)
		{
			// Test if replacement is needed
			if (cursor[0] == '&')
			{
				switch (cursor[1])
				{
					// &amp; &apos;
					case 'a':
					{
						if (cursor[2] == 'm' && cursor[3] == 'p' && cursor[4] == ';')
						{
							out.Append('&');
							cursor += 5;
							continue;
						}
						if (cursor[2] == 'p' && cursor[3] == 'o' && cursor[4] == 's' && cursor[5] == ';')
						{
							out.Append('\'');
							cursor += 6;
							continue;
						}
					} break;

					// &quot;
					case 'q':
					{
						if (cursor[2] == 'u' && cursor[3] == 'o' && cursor[4] == 't' && cursor[5] == ';')
						{
							out.Append('"');
							cursor += 6;
							continue;
						}
					} break;

					// &gt;
					case 'g':
					{
						if (cursor[2] == 't' && cursor[3] == ';')
						{
							out.Append('>');
							cursor += 4;
							continue;
						}
					} break;

					// &lt;
					case 'l':
					{
						if (cursor[2] == 't' && cursor[3] == ';')
						{
							out.Append('<');
							cursor += 4;
							continue;
						}
					} break;

					// Something else
					default:
						// Ignore, just copy '&' verbatim
						break;
				}
			}
		}

		// If whitespace condensing is enabled
		if (normalize_whitespaces)
		{
			// Test if condensing is needed                 
			if (Lookup::skWhitespace[cursor[0]])
			{
				out.Append(' ');
				cursor++; // Skip first whitespace char
					      // Skip remaining whitespace chars
				while (Lookup::skWhitespace[cursor[0]])
				{
					cursor++;
				}
				continue;
			}
		}

		// No replacement, only copy character
		cursor++;
		out.Append(cursor[0]);
	}

	// Return the value
	return out;
}

//----------------------------------------------------------------------------->
// Writes specified node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WriteNode(OutputStream& stream,
                                  const Tree<text::Node>& node,
                                  uint32 depth)
{
	switch (node.data.GetType())
	{
		case text::node::general:         return WriteGeneralNode(stream, node, depth);
		case text::node::comment:         return WriteComment(stream, node, depth);
		case text::node::xml_pi:          return WritePI(stream, node, depth);
		case text::node::xml_doctype:     return WriteDocType(stream, node, depth);
		case text::node::xml_declaration: return WriteDeclaration(stream, node, depth);
		case text::node::xml_cdata:       return WriteCData(stream, node, depth);
		default: return Error(error::not_implemented);
	}
}

// Writes general node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WriteGeneralNode(OutputStream& stream,
                                         const Tree<text::Node>& node,
                                         uint32 depth)
{
	// node attributes
	bool is_attribute = node.data.is_attribute;
	bool has_value = node.data.value;
	bool has_children = node.GetNumChildren() != 0;

	// can't be attribute here
	UT_ASSERT(!is_attribute);

	// tabulation of the node
	const String tabulation = GenerateTabs(depth);

	// start opening tag
	const String node_name(node.data.name.Length() == 0 ? String("unnamed") : node.data.name);
	stream << tabulation << "<" << node_name;

	// write attributes
	bool has_non_attribute_children = false;
	if (has_children)
	{
		for (size_t i = 0; i < node.GetNumChildren(); i++)
		{
			// skip non-attribute elements
			if (!node[i].data.is_attribute)
			{
				has_non_attribute_children = true;
				continue;
			}

			// add space
			stream << " ";

			// write attribute
			Optional<Error> save_attribute_error = WriteAttribute(stream, node[i]);
			if (save_attribute_error)
			{
				return save_attribute_error;
			}
		}
	}

	// end opening tag
	bool is_empty = !has_value && !has_non_attribute_children;
	if (is_empty)
	{
		stream << " /";
	}
	stream << ">";

	// write value
	if (has_value)
	{
		stream << node.data.value.Get();
	}

	// write child nodes
	if (has_children)
	{
		// bool for the first found element node
		bool found_element_node = false;

		// write children
		for (size_t i = 0; i < node.GetNumChildren(); i++)
		{
			// skip attributes
			if (node[i].data.is_attribute)
			{
				continue;
			}

			// start new line
			if (!found_element_node)
			{
				found_element_node = true;
				stream << CarriageReturn<char>();
			}

			// write element
			Optional<Error> save_child_error = WriteNode(stream, node[i], depth + 1);
			if (save_child_error)
			{
				return save_child_error;
			}
		}

		// add tabulation before closing tag
		if (found_element_node)
		{
			stream << tabulation;
		}
	}

	// close tag
	if (!is_empty)
	{
		stream << "</" << node_name << ">";
	}
	
	// new line
	stream << CarriageReturn<char>();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes comment node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WriteComment(OutputStream& stream,
                                     const Tree<text::Node>& node,
                                     uint32 depth)
{
	// must be comment
	UT_ASSERT(node.data.GetType() == text::node::comment);

	// tabulation of the node
	const String tabulation = GenerateTabs(depth);

	// start tag
	stream << tabulation << "<!--";

	// write value
	if (node.data.value)
	{
		stream << node.data.value.Get();
	}

	// end tag
	stream << "-->" << CarriageReturn<char>();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes xml declaration node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WriteDeclaration(OutputStream& stream,
                                         const Tree<text::Node>& node,
                                         uint32 depth)
{
	// must be declaration
	UT_ASSERT(node.data.GetType() == text::node::xml_declaration);

	// tabulation of the node
	const String tabulation = GenerateTabs(depth);

	// start tag
	stream << tabulation << "<?xml";

	// write attributes
	for (size_t i = 0; i < node.GetNumChildren(); i++)
	{
		// must be attribute
		UT_ASSERT(node[i].data.is_attribute);

		// add space
		stream << " ";

		// write attribute
		Optional<Error> save_attribute_error = WriteAttribute(stream, node[i]);
		if (save_attribute_error)
		{
			return save_attribute_error;
		}
	}

	// end tag
	stream << "?>" << CarriageReturn<char>();

	// success
	return Optional<Error>();
}

// Writes cdata node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WriteCData(OutputStream& stream,
	                               const Tree<text::Node>& node,
	                               uint32 depth)
{
	// must be cdata
	UT_ASSERT(node.data.GetType() == text::node::xml_cdata);

	// tabulation of the node
	const String tabulation = GenerateTabs(depth);

	// start tag
	stream << tabulation << "<![CDATA[";

	// write values
	if (node.data.value)
	{
		stream << node.data.value.Get();
	}

	// end tag
	stream << "]]>" << CarriageReturn<char>();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes doctype node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WriteDocType(OutputStream& stream,
                                     const Tree<text::Node>& node,
                                     uint32 depth)
{
	// must be doctype
	UT_ASSERT(node.data.GetType() == text::node::xml_doctype);

	// tabulation of the node
	const String tabulation = GenerateTabs(depth);

	// start tag
	stream << tabulation << "<!DOCTYPE ";

	// write values
	if (node.data.value)
	{
		stream << node.data.value.Get();
	}

	// end tag
	stream << ">" << CarriageReturn<char>();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes process instruction node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WritePI(OutputStream& stream,
	                            const Tree<text::Node>& node,
	                            uint32 depth)
{
	// must be process instruction
	UT_ASSERT(node.data.GetType() == text::node::xml_pi);

	// tabulation of the node
	const String tabulation = GenerateTabs(depth);

	// start tag
	stream << tabulation << "<?" << node.data.name << " ";

	// write values
	if (node.data.value)
	{
		stream << node.data.value.Get();
	}

	// end tag
	stream << "?>" << CarriageReturn<char>();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes specified attribute node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @return - ut::Error if encountered an error
Optional<Error> XmlDoc::WriteAttribute(OutputStream& stream, const Tree<text::Node>& node)
{
	// write name
	stream << node.data.name;

	// write = and opening quotes
	stream << "=\"";

	// write first existing value
	if (node.data.value)
	{
		stream << node.data.value.Get();
	}

	// write closing quotes
	stream << "\"";

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//