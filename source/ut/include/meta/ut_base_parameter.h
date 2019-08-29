//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_archive.h"
#include "system/ut_endianness.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::skLogSerializationEvents static variable indicates if exceptional
// serialization events must be logged to the global log (ut::log).
static const bool skLogSerializationEvents = true;

//----------------------------------------------------------------------------//
// ut::skSerializationOrder static variable defines an order of bytes of the
// variable to be serialized (big-endian or little-endian).
static const endian::order skSerializationEndianness = endian::little;

//----------------------------------------------------------------------------//
// ut::BaseParameter is abstract class to serialize custom data. Every
// serializable structure should be derived from this interface and implement
// two functions: ut::BaseParameter::Save() and ut::BaseParameter::Load().
class BaseParameter : public Archive
{
public:
	// type for size information (number of parameters, parameter size, etc.)
	typedef uint32 SizeType;

	// Constructor
	//    @param p - pointer to the serializable data
	BaseParameter(void* p);

	// Returns the name of the managed type
	virtual String GetTypeName() const = 0;

	// Writes managed object data to the stream. Must be implemented by
	// derived class (child).
	//    @param stream - data will be written to this stream
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(OutputStream& stream) = 0;

	// Loads managed object data from the stream. Must be implemented by
	// derived class (child).
	//    @param stream - data will be loaded from this stream
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(InputStream& stream) = 0;

	// Writes managed object data to the text node. Returns ut::error::not_implemented
	// if it's not overriden by the child class. So you have to imlement it in the
	// derived class to be able to serialize parameter as a text data.
	//    @param node - text node to contain the managed data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(Tree<text::Node>& node);

	// Loads managed object data from the text node. Returns ut::error::not_implemented
	// if it's not overriden by the child class. So you have to imlement it in the
	// derived class to be able to serialize parameter as a text data.
	//    @param node - text node containing the managed data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(const Tree<text::Node>& node);

	// Sets a type name as the node name, but only if this node is unnamed yet.
	//    @param node - node which name to be set
	//    @return - ut::Error if encountered an error
	void SetTextNodeName(Tree<text::Node>& node) const;
protected:

	// Extracts the final value of the provided node
	Result<String, Error> ExtractValueFromTextNode(const Tree<text::Node>& node) const;

	// pointer to the managed object
	void* ptr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//