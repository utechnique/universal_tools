//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_named_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::MetaRegistry is a class for serializing reflective data.
// Every parameter in the registry has own unique name, so it's possible
// to read these parameters from the source in random order. Also parameter
// will be skipped if it failed loading from the source. This allows to
// circumvent versioning problem with the higher memory and performance cost.
class MetaRegistry : public Archive, public NonCopyable
{
public:
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

	// Searches for the parameter with the specified name
	//    @param parameter_name - name of the parameter to be found
	//    @return - reference to the found parameter or nothing
	Optional< Ref<NamedParameter> > FindParameter(const String& parameter_name);

	// Serializes argument object
	//    @param ref - reference to the object to be serialized
	template<typename T>
	Optional<Error> Add(T& ref, const String& name)
	{
		// check if parameter with this name already exists
		if (FindParameter(name))
		{
			return Error(error::already_exists);
		}

		// add new parameter
		UniquePtr<NamedParameter> parameter(new NamedParameter(ref, name));
		parameters.Add(Move(parameter));

		// success
		return Optional<Error>();
	}

private:
	// array of reflective parameters
	Array< UniquePtr<NamedParameter> > parameters;

	// type definition for size information
	// (number of parameters, parameter size, etc.)
	typedef BaseParameter::SizeType SizeType;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//