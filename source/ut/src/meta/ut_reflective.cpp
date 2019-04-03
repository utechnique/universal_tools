//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_reflective.h"
#include "dbg/ut_log.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Writes serialized parameters to the stream.
//    @param stream - data will be written to this stream
//    @return - ut::Error if encountered an error
Optional<Error> Reflective::Save(OutputStream& stream)
{
	MetaRegistry registry;
	Register(registry);
	return registry.Save(stream);
}

// Loads serialized parameters from the stream.
//    @param stream - data will be loaded from this stream
//    @return - ut::Error if encountered an error
Optional<Error> Reflective::Load(InputStream& stream)
{
	MetaRegistry registry;
	Register(registry);
	return registry.Load(stream);
}

// Writes managed object data to the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node to contain the managed data
//    @return - ut::Error if encountered an error
Optional<Error> Reflective::Save(Tree<text::Node>& node)
{
	MetaRegistry registry;
	Register(registry);
	return registry.Save(node);
}

// Loads managed object data from the text node. Returns ut::error::not_implemented
// if it's not overriden by the child class. So you have to imlement it in the
// derived class to be able to serialize parameter as a text data.
//    @param node - text node containing the managed data
//    @return - ut::Error if encountered an error
Optional<Error> Reflective::Load(const Tree<text::Node>& node)
{
	MetaRegistry registry;
	Register(registry);
	return registry.Load(node);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//