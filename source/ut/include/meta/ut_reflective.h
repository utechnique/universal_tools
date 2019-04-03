//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_meta_registry.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Reflective is an abstract class to provide reflection of it's contents.
// Classes inherited from ut::Reflective must implement Register(MetaRegistry&)
// method to serialize(register) member parameters. This class allows
// non-sequential (random) order of parameters for loading. Also invalid
// parameters will be skipped during the loading instead of returning an error.
class Reflective : public Archive
{
public:
	// Registers named parameters to the registry.
	//    @param registry - register parameters here,
	//                      just call registry.Add(parameter, "desired_name");
	virtual void Register(MetaRegistry& registry) = 0;

	// Writes serialized parameters to the stream.
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(OutputStream& stream);

	// Loads serialized parameters from the stream.
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(InputStream& stream);

	// Writes managed object data to the text node.
	//    @param node - text node to contain the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Tree<text::Node>& node);

	// Loads managed object data from the text node.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(const Tree<text::Node>& node);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//