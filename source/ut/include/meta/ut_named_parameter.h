//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_parameter_deduction.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::NamedParameter class is a parameter with the name information.
// It is used in ut::MetaRegistry class to perform non-sequential loading,
// so that parameters with unknown names could be skipped.
class NamedParameter : public BaseParameter, public NonCopyable
{
public:
	// Template constructor, object creation is expected to be without <> braces,
	// so that @ref parameter was deducted automatically. Example:
	// 1> String var;
	// 2> UniquePtr<NamedParameter> parameter(new NamedParameter(var, "name"));
	//    @param p - pointer to the managed object
	template<typename T>
	NamedParameter(T& ref, const String& parameter_name) : BaseParameter(&ref)
	                                                     , name(parameter_name)
	                                                     , parameter(DeduceParameter(ref))
	{ }

	// Returns the name of the parameter
	const String& GetName() const;

	// Returns the name of the managed type
	String GetTypeName() const;

	// Writes managed data to the stream
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(OutputStream& stream);

	// Loads managed data from the stream
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

private:
	// name of the parameter, must be provided by constructor
	String name;

	// parameter of the corresponding type, supposed to be
	// created in the appropriate template constructor
	UniquePtr<BaseParameter> parameter;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//