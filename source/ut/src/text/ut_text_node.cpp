//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "text/ut_document.h"
#include "streams/ut_file.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(text)
//----------------------------------------------------------------------------//
// Constructor, takes node type as an argument
//    @param node_type - type of the node
Node::Node(node::Type node_type) : is_attribute(false)
                                 , is_array(false)
                                 , type(node_type)
{ }

// Copy constructor
Node::Node(const Node& copy) : name(copy.name)
                             , value(copy.value)
                             , value_type(copy.value_type)
                             , encapsulation_name(copy.encapsulation_name)
                             , is_attribute(copy.is_attribute)
                             , is_array(copy.is_array)
                             , type(copy.type)
{ }

// Move constructor
#if CPP_STANDARD >= 2011
Node::Node(Node && copy) : name(Move(copy.name))
                         , value(Move(copy.value))
                         , value_type(Move(copy.value_type))
                         , encapsulation_name(Move(copy.encapsulation_name))
                         , is_attribute(Move(copy.is_attribute))
                         , is_array(copy.is_array)
                         , type(copy.type)
{ }
#endif

// Assignment operator
Node& Node::operator = (const Node& copy)
{
	name = copy.name;
	value = copy.value;
	value_type = copy.value_type;
	encapsulation_name = copy.encapsulation_name;
	is_attribute = copy.is_attribute;
	is_array = copy.is_array;
	type = copy.type;
	return *this;
}

// Move operator
#if CPP_STANDARD >= 2011
Node& Node::operator = (Node && copy)
{
	name = Move(copy.name);
	value = Move(copy.value);
	value_type = Move(copy.value_type);
	encapsulation_name = Move(copy.encapsulation_name);
	is_attribute = copy.is_attribute;
	is_array = copy.is_array;
	type = copy.type;
	return *this;
}
#endif

// Returns a type of the node
node::Type Node::GetType() const
{
	return type;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(text)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//