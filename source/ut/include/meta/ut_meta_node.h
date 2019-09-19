//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "pointers/ut_shared_ptr.h"
#include "meta/ut_meta_base_parameter.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Node is a class-container for meta data.
class Node
{
public:
	// Default constructor
	Node();

	// Constructor
	//    @param in_parameter - shared pointer to the parameter
	//                          that represents a reflected object
	//    @param in_name - name of the parameter
	//    @param in_id - id of the parameter
	Node(const SharedPtr<BaseParameter>& in_parameter,
	     const String& in_name,
		 uint32 in_id);

	// Constructor
	//    @param in_parameter - shared pointer to a parameter
	//                          that represents a reflected object
	//    @param in_name - r-value reference to the name of the parameter
	//    @param in_id - id of the parameter
#if CPP_STANDARD >= 2011
	Node(const SharedPtr<BaseParameter>& in_parameter,
	     String&& in_name,
		 uint32 in_id);
#endif

	// Copy constructor
	Node(const Node& copy);

	// Move constructor
#if CPP_STANDARD >= 2011
	Node(Node&& ref);
#endif

	// shared pointer to the parameter that represents a reflected object
	SharedPtr<BaseParameter> parameter;

	// name of the parameter
	String name;

	// identifier of the parameter
	uint32 id;
};

//----------------------------------------------------------------------------//

namespace node_names
{
	static const char* skValue = "value";
	static const char* skType = "type";
	static const char* skCount = "count";
	static const char* skValueType = "value_type";
	static const char* skId = "id";
	static const char* skInfo = "info";
	static const char* skVersion = "version";
	static const char* skFlags = "flags";
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//