//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_base_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Constructor
//    @param p - pointer to the serializable data
BaseParameter::BaseParameter(void* p) : ptr(p)
{
	UT_ASSERT(p != nullptr);
}

// Writes managed object data to the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node to contain the managed data
//    @return - ut::Error if encountered an error
Optional<Error> BaseParameter::Save(Tree<text::Node>& node)
{
	return Archive::Save(node);
}

// Loads managed object data from the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node containing the managed data
//    @return - ut::Error if encountered an error
Optional<Error> BaseParameter::Load(const Tree<text::Node>& node)
{
	return Archive::Load(node);
}

// Sets a type name as the node name, but only if this node is unnamed yet.
//    @param node - node which name to be set
//    @return - ut::Error if encountered an error
void BaseParameter::SetTextNodeName(Tree<text::Node>& node) const
{
	if (node.data.name.Length() == 0)
	{
		node.data.name = GetTypeName();
	}
}

// Extracts the final value of the provided node
Result<String, Error> BaseParameter::ExtractValueFromTextNode(const Tree<text::Node>& node) const
{
	// scan element from the text
	if (node.data.value)
	{
		return node.data.value.Get();
	}

	// some document formats do not allow simultaneous existance of
	// child nodes and values, thus we must check if there is special
	// value node with special name
	if (node.GetNumChildren() != 0)
	{
		for (size_t i = 0; i < node.GetNumChildren(); i++)
		{
			// value node must have special name
			if (node[i].data.name != text::Document::skValueNodeName)
			{
				continue;
			}

			// check if the node has a value
			if (!node[i].data.value)
			{
				String description("Provided text node has no values but has ");
				description += "a suitable child node, however this node has no values.";
				return MakeError(Error(error::invalid_arg, Move(description)));
			}

			// return a value
			return node[i].data.value.Get();
		}
	}

	// fail
	String description("Provided text node has no values and no suitable child node.");
	return MakeError(Error(error::invalid_arg, Move(description)));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//