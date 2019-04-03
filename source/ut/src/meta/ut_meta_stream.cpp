//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_stream.h"
#include "meta/ut_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Constructor
MetaStream::MetaStream()
{ }

//----------------------------------------------------------------------------->
// Writes serialized parameters to the stream.
//    @param stream - data will be written to this stream
//    @return - ut::Error if encountered an error
Optional<Error> MetaStream::Save(OutputStream& stream)
{
	for (size_t i = 0; i < parameters.GetNum(); i++)
	{
		Optional<Error> save_error = parameters[i]->Save(stream);
		if (save_error)
		{
			return save_error;
		}
	}

	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Loads serialized parameters from the stream.
//    @param stream - data will be loaded from this stream
//    @return - ut::Error if encountered an error
Optional<Error> MetaStream::Load(InputStream& stream)
{
	for (size_t i = 0; i < parameters.GetNum(); i++)
	{
		Optional<Error> load_error = parameters[i]->Load(stream);
		if (load_error)
		{
			return load_error;
		}
	}

	return Optional<Error>();
}

// Writes managed object data to the text node.
//    @param node - text node to contain the managed data
//    @return - ut::Error if encountered an error
Optional<Error> MetaStream::Save(Tree<text::Node>& node)
{
	node.data.is_array = true;
	for (size_t i = 0; i < parameters.GetNum(); i++)
	{
		// create and fill node for the parameter
		Tree<text::Node> element_node;
		Optional<Error> save_error = parameters[i]->Save(element_node);
		if (save_error)
		{
			return save_error;
		}

		// add parameter node to the parent node
		if (!node.Add(element_node))
		{
			return Error(error::out_of_memory);
		}
	}

	// success
	return Optional<Error>();
}

// Loads managed object data from the text node.
//    @param node - text node containing the managed data
//    @return - ut::Error if encountered an error
Optional<Error> MetaStream::Load(const Tree<text::Node>& node)
{
	// validate the number of child nodes
	if (node.GetNumChildren() < parameters.GetNum())
	{
		return Error(error::fail, "Text node has too few children for this metastream object.");
	}

	// load every parameter sequentially
	for (size_t i = 0; i < parameters.GetNum(); i++)
	{
		Optional<Error> load_error = parameters[i]->Load(node[i]);
		if (load_error)
		{
			return load_error;
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//