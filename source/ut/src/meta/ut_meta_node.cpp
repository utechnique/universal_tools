//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_node.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Default constructor
Node::Node() : id(0)
{ }

// Constructor
//    @param in_parameter - shared pointer to a parameter
//                          that represents a reflected object
//    @param in_name - name of the parameter
//    @param in_id - id of the parameter
Node::Node(const SharedPtr<BaseParameter>& in_parameter,
           const String& in_name,
           uint32 in_id) : parameter(in_parameter)
                         , name(in_name)
                         , id(in_id)
{ }

// Constructor
//    @param in_parameter - shared pointer to a parameter
//                          that represents a reflected object
//    @param in_name - r-value reference to the name of the parameter
//    @param in_id - id of the parameter
#if CPP_STANDARD >= 2011
Node::Node(const SharedPtr<BaseParameter>& in_parameter,
           String&& in_name,
           uint32 in_id) : parameter(in_parameter)
                         , name(Move(in_name))
                         , id(in_id)
{ }
#endif

// Copy constructor
Node::Node(const Node& copy) : parameter(copy.parameter)
                             , name(copy.name)
                             , id(copy.id)
{ }

// Move constructor
#if CPP_STANDARD >= 2011
Node::Node(Node&& ref) : parameter(Move(ref.parameter))
                       , name(Move(ref.name))
                       , id(Move(ref.id))
{ }
#endif

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//