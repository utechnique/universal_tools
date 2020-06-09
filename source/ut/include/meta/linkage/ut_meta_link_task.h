//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "pointers/ut_shared_ptr.h"
#include "meta/ut_meta_controller.h"
#include "meta/linkage/ut_meta_link.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Forward declarations.
class Linker;
//----------------------------------------------------------------------------//
// ut::meta::LinkTask is an abstract class that is a base class for different
// linkinkg tasks.
class LinkTask
{
public:
	virtual Optional<Error> Execute(Linker& linker) = 0;
	virtual ~LinkTask() = default;
};

//----------------------------------------------------------------------------//
// Holds controller state with the position where id of the linked parameter
// must be written and adress of that linked parameter. This tasks searches
// for the linked parameter and overwrites it's id on execution.
class WriteLinkTask : public LinkTask
{
public:
	// Constructor
	//    @param in_state - state of the controller that represents a node
	//                      of the parameter holding a link. Id of the linked
	//                      parameter will be written here on execution.
	//    @param in_address - address of the linked parameter.
	WriteLinkTask(Controller in_state,
	              const void* in_address);

	// Searches for the linked parameter and then writes it's id using controller.
	//    @param linker - reference to the linker object.
	//    @return - error if failed.
	Optional<Error> Execute(Linker& linker);

private:
	Controller state;
	const void* linked_address;
};

//----------------------------------------------------------------------------//
// Holds pointer to the parameter that represents a link and id of the parameter
// that must be linked with it. Searches for the linked parameter and calls it's
// ut::meta::Parameter::Link() method on execution.
class ReadLinkTask : public LinkTask
{
public:
	// Constructor
	//    @param in_parameter - pointer to the parameter that represents a link.
	//    @param in_id - id of the linked parameter.
	ReadLinkTask(const BaseParameter* in_parameter,
	             size_t in_id);

	// Searches for the linked parameter (but in special 'shared' pool) and then
    // calls ut::meta::Parameter::Link() method with this parameter as an
	// argument.
	//    @param linker - reference to the linker object.
	//    @return - error if failed.
	virtual Optional<Error> Execute(Linker& linker);

protected:
	const BaseParameter* parameter;
	size_t destination_id;
};

//----------------------------------------------------------------------------//
// ut::meta::ReadSharedPtrLinkTask is a variation of the ut::meta::ReadLinkTask
// but for shared objects. It calls ut::meta::Parameter::Link() method of the
// managed parameter, but passes a pointer to ut::SharedPtr smart pointer
// container but not to the object itself.
class ReadSharedPtrLinkTask : public ReadLinkTask
{
public:
	// Constructor
	//    @param in_parameter - pointer to the parameter that represents a link.
	//    @param in_id - id of the linked parameter.
	ReadSharedPtrLinkTask(const BaseParameter* in_parameter,
	                      size_t in_id);

	// Searches for the linked parameter and then calls it's
	// ut::meta::Parameter::Link() method
	//    @param linker - reference to the linker object.
	//    @return - error if failed.
	Optional<Error> Execute(Linker& linker);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//