//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_named_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Returns the name of the parameter
const String& NamedParameter::GetName() const
{
	return name;
}

// Returns the name of the managed type
String NamedParameter::GetTypeName() const
{
	return parameter->GetTypeName();
}

// Writes managed data to the stream
//    @param stream - data will be written to this stream
//    @return - ut::Error if encountered an error
Optional<Error> NamedParameter::Save(OutputStream& stream)
{
	// write the name of the type
	String type_name = GetTypeName();
	Parameter<String> typename_parameter(&type_name);
	Optional<Error> save_typename_error = typename_parameter.Save(stream);
	if (save_typename_error)
	{
		return save_typename_error;
	}

	// write the parameter itself
	return parameter->Save(stream);
}

// Loads managed data from the stream
//    @param stream - data will be loaded from this stream
//    @return - ut::Error if encountered an error
Optional<Error> NamedParameter::Load(InputStream& stream)
{
	// read parameter typename
	String parameter_type;
	Parameter<String> typename_parameter(&parameter_type);
	Optional<Error> load_typename_error = typename_parameter.Load(stream);
	if (load_typename_error)
	{
		return load_typename_error;
	}

	// validate the parameter
	if (parameter_type != GetTypeName())
	{
		return Error(error::types_not_match);
	}

	// load the actual parameter
	return parameter->Load(stream);
}

// Writes managed object data to the text node.
//    @param node - text node to contain the managed data
//    @return - ut::Error if encountered an error
Optional<Error> NamedParameter::Save(Tree<text::Node>& node)
{
	// set the name of the type
	node.data.name = name;

	// save base parameter
	return parameter->Save(node);
}

// Loads managed object data from the text node.
//    @param node - text node containing the managed data
//    @return - ut::Error if encountered an error
Optional<Error> NamedParameter::Load(const Tree<text::Node>& node)
{
	// validate name
	if (node.data.name != name)
	{
		return Error(error::fail, "Reflective parameter name doesn't match the node value.");
	}

	// load the parameter
	return parameter->Load(node);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//