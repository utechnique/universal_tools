//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_parameter_deduction.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::MetaStream is a class to serialize objects. Uses temporary buffer of so
// called serialization parameters. Parameter is not an object itself, but a
// proxy structure, that knows how to save or load managed object. Use operator
// '<<' to serialize any data. Simple and fundamental types can be serialized
// explicitly, comlex types must be derived from ut::Serializable and implement
// Serialize() method to form serialization tree.
class MetaStream : public Archive
{
public:
	// Constructor
	MetaStream();

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

	// Serializes argument object
	//    @param ref - reference to the object to be serialized
	template<typename T>
	MetaStream& operator << (T& ref)
	{
		// Add one of the possible parameters - correct
		// variant is deduced from the argument list
		UniquePtr<BaseParameter> parameter(DeduceParameter(ref));
		parameters.Add(Move(parameter));
		return *this;
	}

protected:
	// array of parameters
	Array< UniquePtr<BaseParameter> > parameters;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//