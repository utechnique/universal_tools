//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "text/ut_json.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Parses raw text
//    @param text - string with a text to be parsed
//    @return - ut::Error if encountered an error
Optional<Error> JsonDoc::Parse(const String& doc)
{
	// create input object with the address of the provided string
	text::Reader cursor(doc.GetAddress());
	
	// remove current contents
	nodes.Reset();

	// check if document is empty
	Skip(cursor, Lookup::skWhitespace);
	if (cursor[0] == '\0')
	{
		return Error(error::empty);
	}

	// create JSON node
	Tree<text::Node> JSON;
	JSON.data.name = "JSON";

	// parse value node
	Optional<Error> parse_error = ParseValue(cursor, JSON);
	if (parse_error)
	{
		return parse_error;
	}

	// add child nodes, JSON node is not included!
	for (size_t i = 0; i < JSON.CountChildren(); i++)
	{
		if (!nodes.Add(Move(JSON[i])))
		{
			return Error(error::out_of_memory);
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Writes contents to the output stream
//    @param stream - output stream
//    @return - ut::Error if encountered an error
Optional<Error> JsonDoc::Write(OutputStream& stream) const
{
	// write opening brace
	stream << "{" << CarriageReturn<char>();

	// write all nodes
	const size_t node_num = nodes.Count();
	for (size_t i = 0; i < node_num; i++)
	{
		// write the node
		Optional<Error> write_error = WriteNode(stream, nodes[i], true, 1);
		if (write_error)
		{
			return write_error;
		}

		// insert ',' symbol
		if (i != node_num - 1)
		{
			stream << ",";
		}

		// new line
		stream << CarriageReturn<char>();
	}

	// write closing brace
	stream << "}";

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Parses the value of the node
//    @param cursor - reference to the current parsing position
//    @param node - reference to the parent node
//    @return - ut::Error if encountered an error
Optional<Error> JsonDoc::ParseValue(text::Reader& cursor, Tree<text::Node>& node)
{
	// Is it a string?
	if (cursor == '"')
	{
		cursor++;
		Result<String, Error> string_result = ExtractString(cursor);
		if (!string_result)
		{
			return string_result.MoveAlt();
		}
		node.data.value = string_result.Move();
	}
	else if (cursor.CheckLength(4) && cursor.Compare("true", false)) // true?
	{
		cursor += 4;
		node.data.value_type = String(Type<bool>::Name());
		node.data.value = String("true");
	}
	else if (cursor.CheckLength(5) && cursor.Compare("false", false)) // false?
	{
		cursor += 5;
		node.data.value_type = String(Type<bool>::Name());
		node.data.value = String("false");
	}
	else if (cursor.CheckLength(4) && cursor.Compare("null", false)) // null?
	{
		cursor += 4;
		node.data.value_type = String(Type<int>::Name());
		node.data.value = String("null");
	}
	else if (cursor == L'-' || (cursor[0] >= L'0' && cursor[0] <= L'9')) // number?
	{
		Result<String, Error> number_result = ExtractNumber(cursor);
		if (!number_result)
		{
			return number_result.MoveAlt();
		}

		node.data.value_type = String(Type<int>::Name());
		node.data.value = number_result.Move();
	}
	else if (cursor == '{') // object?
	{
		Optional<Error> object_error = ParseObject(cursor, node);
		if (object_error)
		{
			return object_error;
		}
	}
	else if (cursor == '[') // array?
	{
		Optional<Error> array_error = ParseArray(cursor, node);
		if (array_error)
		{
			return array_error;
		}
	}
	else
	{
		// something wrong
		return Error(error::fail, "Unknown value type.");
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Parses '{}' object node
//    @param cursor - reference to the current parsing position
//    @param node - reference to the parent node
//    @return - ut::Error if encountered an error
Optional<Error> JsonDoc::ParseObject(text::Reader& cursor, Tree<text::Node>& node)
{
	// every json object must start with '{' character
	UT_ASSERT(cursor == '{');

	// skip '{'
	cursor++;

	while (cursor != 0)
	{
		// We are parsing a child node here
		Tree<text::Node> child_node;

		// Whitespace at the start?
		Skip(cursor, Lookup::skWhitespace);
		if (cursor == '\0')
		{
			return Error(error::fail, "Unexpected end of file.");
		}

		// Special case - empty object
		if (!node.data.value && node.CountChildren() == 0 && cursor == '}')
		{
			cursor++;
			return Optional<Error>();
		}

		// We want a string now...
		if (cursor != '\"')
		{
			return Error(error::fail, "JSON object has no name.");
		}

		// Read the name
		cursor++;
		Result<String, Error> name_result = ExtractString(cursor);
		if (name_result)
		{
			child_node.data.name = name_result.Move();
		}
		else
		{
			return name_result.MoveAlt();
		}

		// More whitespace?
		Skip(cursor, Lookup::skWhitespace);
		if (cursor == '\0')
		{
			return Error(error::fail, "Unexpected end of file.");
		}

		// Need a : now
		if (cursor != L':')
		{
			return Error(error::fail, "JSON object - expected \":\" character.");
		}
		cursor++;

		// More whitespace?
		Skip(cursor, Lookup::skWhitespace);
		if (cursor == '\0')
		{
			return Error(error::fail, "Unexpected end of file.");
		}

		// The value is here
		Optional<Error> parse_value_error = ParseValue(cursor, child_node);
		if (parse_value_error)
		{
			return parse_value_error;
		}

		// More whitespace?
		Skip(cursor, Lookup::skWhitespace);
		if (cursor == '\0')
		{
			return Error(error::fail, "Unexpected end of file.");
		}

		// End of object?
		if (cursor == '}')
		{
			cursor++;

			// Add the child node
			if (!node.Add(Move(child_node)))
			{
				return Error(error::out_of_memory);
			}

			break;
		}

		// Want a ',' now
		if (cursor != ',')
		{
			return Error(error::fail, "JSON object - expected \",\" character.");
		}

		// Add the child node
		if (!node.Add(Move(child_node)))
		{
			return Error(error::out_of_memory);
		}

		// To the next character
		cursor++;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Parses '[]' array node
//    @param cursor - reference to the current parsing position
//    @param node - reference to the parent node
//    @return - ut::Error if encountered an error
Optional<Error> JsonDoc::ParseArray(text::Reader& cursor, Tree<text::Node>& node)
{
	// every json object must start with '[' character
	UT_ASSERT(cursor == '[');

	// skip '['
	cursor++;

	// set appropriate array flag
	node.data.is_array = true;

	while (cursor != '\0')
	{
		// We are parsing an array element node here
		Tree<text::Node> element_node;

		// Whitespace at the start?
		Skip(cursor, Lookup::skWhitespace);
		if (cursor == '\0')
		{
			return Error(error::fail, "Unexpected end of file.");
		}

		// Special case - empty array
		if (node.CountChildren() == 0 && cursor == ']')
		{
			cursor++;
			return Optional<Error>();
		}

		// Get the value
		Optional<Error> parse_value_error = ParseValue(cursor, element_node);
		if (parse_value_error)
		{
			return parse_value_error;
		}

		// Add the element node
		if (!node.Add(Move(element_node)))
		{
			return Error(error::out_of_memory);
		}

		// More whitespace?
		Skip(cursor, Lookup::skWhitespace);
		if (cursor == '\0')
		{
			return Error(error::fail, "Unexpected end of file.");
		}

		// End of array?
		if (cursor == ']')
		{
			cursor++;
			break;
		}

		// Want a , now
		if (cursor != ',')
		{
			return Error(error::fail, "JSON object - expected \",\" character.");
		}

		// To the next character
		cursor++;
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Extracts a JSON String as defined by the spec - "<some chars>"
// Any escaped characters are swapped out for their unescaped values
//    @param cursor - reference to the current parsing position
//    @return - extracted string or ut::Error if encountered an error
Result<String, Error> JsonDoc::ExtractString(text::Reader& cursor)
{
	String str;

	while (cursor != '\0')
	{
		// Save the char so we can change it if need be
		char next_char = cursor[0];

		// Escaping something?
		if (next_char == '\\')
		{
			// Move over the escape char
			cursor++;

			// Deal with the escaped char
			switch (cursor[0])
			{
			case '"':  next_char = '"';  break;
			case '\\': next_char = '\\'; break;
			case '/':  next_char = '/';  break;
			case 'b':  next_char = '\b'; break;
			case 'f':  next_char = '\f'; break;
			case 'n':  next_char = '\n'; break;
			case 'r':  next_char = '\r'; break;
			case 't':  next_char = '\t'; break;
			case 'u':
			{
				// We need 5 chars (4 hex + the 'u') or its not valid
				if (!cursor.CheckLength(5))
				{
					return MakeError(error::fail, "\\u sequence is invalid: must be 4 hex + the 'u'.");
				}

				// Deal with the chars
				next_char = 0;
				for (int i = 0; i < 4; i++)
				{
					// Do it first to move off the 'u' and leave us on the
					// final hex digit as we move on by one later on
					cursor++;

					next_char <<= 4;

					// Parse the hex digit
					if (cursor[0] >= '0' && cursor[0] <= '9')
					{
						next_char |= (cursor[0] - '0');
					}
					else if (cursor[0] >= 'A' && cursor[0] <= 'F')
					{
						next_char |= (10 + (cursor[0] - 'A'));
					}
					else if (cursor[0] >= 'a' && cursor[0] <= 'f')
					{
						next_char |= (10 + (cursor[0] - 'a'));
					}
					else
					{
						// Invalid hex digit = invalid JSON
						return MakeError(error::fail, "Invalid hex digit.");
					}
				}
				break;
			}

			// By the spec, only the above cases are allowed
			default:
				return MakeError(error::fail, "Invalid string.");
			}
		}
		else if (next_char == L'"') // End of the string?
		{
			cursor++;
			return str;
		}
		else if (next_char < L' ' && next_char != L'\t') // Disallowed char?
		{
			// SPEC Violation: Allow tabs due to real world cases
			return MakeError(error::fail, "Disallowed character in a string.");
		}

		// Add the next char
		str.Append(next_char);

		// Move on
		cursor++;
	}

	// If we're here, the string ended incorrectly
	return MakeError(error::fail, "Unexpected end of the string.");
}

//----------------------------------------------------------------------------->
// Extracts a JSON digit
//    @param cursor - reference to the current parsing position
//    @return - extracted string or ut::Error if encountered an error
Result<String, Error> JsonDoc::ExtractNumber(text::Reader& cursor)
{
	// First symbol
	const char* start = cursor.Get();

	// Negative?
	bool neg = cursor == '-';
	if (neg)
	{
		cursor++;
	}

	double number = 0.0;

	// Parse the whole part of the number - only if it wasn't 0
	if (cursor == '0')
	{
		cursor++;
	}
	else if (cursor[0] >= '1' && cursor[0] <= L'9')
	{
		double integer = 0;
		while (cursor != '\0' && cursor[0] >= '0' && cursor[0] <= '9')
		{
			cursor++;
			integer = integer * 10 + (cursor[0] - '0');
		}
	}
	else
	{
		return MakeError(error::fail, "Invalid digit.");
	}

	// Could be a decimal now...
	if (cursor == '.')
	{
		cursor++;

		// Not get any digits?
		if (!(cursor[0] >= L'0' && cursor[0] <= L'9'))
		{
			return MakeError(error::fail, "Digit has invalid decimal part.");
		}

		// Find the decimal and sort the decimal place out
		// Use ParseDecimal as ParseInt won't work with decimals less than 0.1
		// thanks to Javier Abadia for the report & fix
		double decimal = 0.0;
		double factor = 0.1;
		while (cursor != '\0' && cursor[0] >= '0' && cursor[0] <= '9')
		{
			cursor++;
			int digit = (cursor[0] - '0');
			decimal = decimal + digit * factor;
			factor *= 0.1;
		}

		// Save the number
		number += decimal;
	}

	// Could be an exponent now...
	if (cursor == 'E' || cursor == 'e')
	{
		cursor++;

		// Check signage of expo
		bool neg_expo = false;
		if (cursor == '-' || cursor == '+')
		{
			neg_expo = cursor == '-';
			cursor++;
		}

		// Not get any digits?
		if (!(cursor[0] >= L'0' && cursor[0] <= L'9'))
		{
			return MakeError(error::fail, "Number has invalid exponent.");
		}

		// Sort the expo out
		double expo = 0;
		while (cursor != '\0' && cursor[0] >= '0' && cursor[0] <= '9')
		{
			cursor++;
			expo = expo * 10 + (cursor[0] - '0');
		}

		// Complete the number
		for (double i = 0.0; i < expo; i++)
		{
			number = neg_expo ? (number / 10.0) : (number * 10.0);
		}
	}

	// Was it neg?
	if (neg)
	{
		number *= -1;
	}

	// Last symbol
	const char* end = cursor.Get();

	// Success
	return String(start, static_cast<size_t>(end - start));
}

//----------------------------------------------------------------------------->
// Writes specified node to the output stream
//    @param stream - output stream
//    @param node - node to be written
//    @param depth - callstack recursive depth of the function
//    @return - ut::Error if encountered an error
Optional<Error> JsonDoc::WriteNode(OutputStream& stream,
	                               const Tree<text::Node>& node,
                                   bool write_name,
	                               uint32 depth)
{
	// node attributes
	bool has_value = static_cast<bool>(node.data.value);
	bool has_children = node.CountChildren() != 0;

	// tabulation of the node
	const String tabulation = GenerateTabs(depth);

	// write node name
	stream << tabulation;
	if (write_name)
	{
		stream << "\"" << node.data.name << "\": ";
	}

	if (has_children)
	{
		stream << (node.data.is_array ? "[" : "{");
		stream << CarriageReturn<char>();

		const size_t num_children = node.CountChildren();
		for (size_t i = 0; i < num_children; i++)
		{
			// write child node
			Optional<Error> write_child_error = WriteNode(stream,
			                                              node[i],
			                                              !node.data.is_array,
			                                              depth + 1);
			if (write_child_error)
			{
				return write_child_error;
			}

			// insert ',' symbol
			if ((i != num_children - 1) || has_value)
			{
				stream << ",";
			}

			// carriage return
			stream << CarriageReturn<char>();
		}

		// odd behaviour for json - when the node has values and children simultaneously
		if (has_value)
		{
			// create a separate node for the value
			Tree<text::Node> value_node;
			value_node.data.value = node.data.value;
			value_node.data.value_type = node.data.value_type;

			// set value node name
			if (node.data.encapsulation_name)
			{
				value_node.data.name = node.data.encapsulation_name.Get();
			}
			else
			{
				value_node.data.name = Document::skValueNodeName;
			}

			// write value node
			Optional<Error> write_value_error = WriteNode(stream,
			                                              value_node,
			                                              true,
			                                              depth + 1);
			if (write_value_error)
			{
				return write_value_error;
			}
			stream << CarriageReturn<char>();
		}

		// close braces
		stream << tabulation;
		stream << (node.data.is_array ? "]" : "}");
	}
	else if(has_value)
	{
		bool is_numeric = false;
		bool is_bool = false;
		if (node.data.value_type)
		{
			is_numeric = IsNumericType<char>(node.data.value_type->GetAddress());
			is_bool = node.data.value_type.Get() == Type<bool>::Name();
		}

		if (!is_numeric && !is_bool)
		{
			stream << "\"";
		}

		stream << node.data.value.Get();

		if (!is_numeric && !is_bool)
		{
			stream << "\"";
		}
	}
	else // special case - empty node
	{
		stream << (node.data.is_array ? "[ ]" : "{ }");
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//