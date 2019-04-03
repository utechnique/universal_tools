//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "error/ut_error.h"
#include "streams/ut_input_stream.h"
#include "streams/ut_output_stream.h"
#include "meta/ut_archive.h"
#include "meta/ut_meta_stream.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Serializable is a base class for serializable objects. Custom class can
// be serialized if it's inherited from the ut::Serializable class and
// implemented ut::Serializable::Serialize() method.
class Serializable : public Archive
{
public:
	// Writes serialized parameters to the stream.
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(OutputStream& stream);

	// Loads serialized parameters from the stream.
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(InputStream& stream);

	// Writes managed object data to the text node.
	//    @param node - text node to contain the managed data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(Tree<text::Node>& node);

	// Loads managed object data from the text node.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(const Tree<text::Node>& node);

	// Serialize your data here after implementing this function in derived
	// classes. Use stream << 'your_object' to serialize members.
	//    @param stream - serialization stream
	virtual void Serialize(MetaStream& stream) = 0;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//