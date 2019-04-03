//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_serializable.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Writes serialized parameters to the stream.
//    @param stream - data will be written to this stream
//    @return - ut::Error if encountered an error
Optional<Error> Serializable::Save(OutputStream& stream)
{
	MetaStream metadata;
	Serialize(metadata);
	return metadata.Save(stream);
}

//----------------------------------------------------------------------------->
// Loads serialized parameters from the stream.
//    @param stream - data will be loaded from this stream
//    @return - ut::Error if encountered an error
Optional<Error> Serializable::Load(InputStream& stream)
{
	MetaStream metadata;
	Serialize(metadata);
	return metadata.Load(stream);
}

//----------------------------------------------------------------------------->
// Writes managed object data to the text node.
//    @param node - text node to contain the managed data
//    @return - ut::Error if encountered an error
Optional<Error> Serializable::Save(Tree<text::Node>& node)
{
	MetaStream metadata;
	Serialize(metadata);
	return metadata.Save(node);
}

//----------------------------------------------------------------------------->
// Loads managed object data from the text node.
//    @param node - text node containing the managed data
//    @return - ut::Error if encountered an error
Optional<Error> Serializable::Load(const Tree<text::Node>& node)
{
	MetaStream metadata;
	Serialize(metadata);
	return metadata.Load(node);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//