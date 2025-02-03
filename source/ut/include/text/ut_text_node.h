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
#include "streams/ut_input_stream.h"
#include "streams/ut_output_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(text)
//----------------------------------------------------------------------------//
// ut::text::node::Type is a namespace containg possible node types for a text document.
// Consider that only ut::node::general is common for all formats.
namespace node
{
	enum class Type
	{
		general,
		comment,
		xml_pi,
		xml_doctype,
		xml_declaration,
		xml_cdata,
	};
}

//----------------------------------------------------------------------------//
// ut::text::Node is collecting class for all possible nodes in any text document.
// It is quite excessive, but fulfills criterions for all text formats. 
class Node
{
public:
	// Constructor, takes node type as an argument
	//    @param node_type - type of the node
	Node(node::Type node_type = node::Type::general);

	// Returns a type of the node
	node::Type GetType() const;

	// name of the node
	String name;

	// value of the node
	Optional<String> value;

	// name of the value type
	Optional<String> value_type;

	// some text document don't support simultaneous existance of child
	// nodes and value (json), thus we must isolate a value in a new node, and this
	// variable (text::Node::encapsulation_name) is a name of such a node, default
	// name (Document::skValueNodeName) will be used if this variable is empty
	Optional<String> encapsulation_name;

	// set to 'true' if this node is an attribute,
	// this member takes effect only for xml documents
	bool is_attribute;

	// set to 'true' if this node is an array
	// this member takes effect only for json documents
	bool is_array;

private:
	// type of this node
	node::Type type;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(text)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//