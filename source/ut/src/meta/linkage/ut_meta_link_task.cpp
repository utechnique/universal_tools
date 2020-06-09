//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/linkage/ut_meta_link_task.h"
#include "meta/linkage/ut_meta_linker.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Constructor
//    @param in_state - state of the controller that represents a node
//                      of the parameter holding a link. Id of the linked
//                      parameter will be written here on execution.
//    @param in_address - address of the linked parameter.
WriteLinkTask::WriteLinkTask(Controller in_state,
                             const void* in_address) : state(in_state)
                                                     , linked_address(in_address)
{}

// Searches for the linked parameter and then writes it's id using controller.
//    @param linker - reference to the linker object.
//    @return - error if failed.
Optional<Error> WriteLinkTask::Execute(Linker& linker)
{
	// search for the linked parameter
	Optional<Link&> dst_link = linker.FindLinkByAddress(linked_address);
	if (!dst_link)
	{
		// add log event
		String error_desc = "There is no associated link for this parameter: ";
		error_desc += Print(linked_address);
		state.GetInfo().LogMessage(error_desc);

		// exit
		return Error(error::not_found, error_desc);
	}

	// write id
	return linker.WriteLinkId(state, dst_link->id);
}

//----------------------------------------------------------------------------//
// Constructor
//    @param in_parameter - pointer to the parameter that represents a link.
//    @param in_id - id of the linked parameter.
ReadLinkTask::ReadLinkTask(const BaseParameter* in_parameter,
                           size_t in_id) : parameter(in_parameter)
                                         , destination_id(in_id)
{}

// Searches for the linked parameter and then calls
// ut::meta::Parameter::Link() method
//    @param linker - reference to the linker object.
//    @return - error if failed.
Optional<Error> ReadLinkTask::Execute(Linker& linker)
{
	// search for the parameter that is being linked
	Optional<Link&> src_result = linker.FindLinkByParameter(parameter);
	if (!src_result)
	{
		// add log event
		String error_desc = "There is no associated link for this parameter: ";
		error_desc += Print(static_cast<const void*>(parameter));

		// exit
		return Error(error::not_found, error_desc);
	}

	// search for the linked parameter
	Optional<Link&> dst_result = linker.FindLinkById(destination_id);
	if (!dst_result)
	{
		// add log event
		String error_desc = "Couldn't find linked parameter with id \"";
		error_desc += Print(src_result->id) + "\" by id: " + Print(destination_id);

		// exit
		return Error(error::not_found, error_desc);
	}

	// link parameters
	Link& src_link = src_result.Get();
	Link& dst_link = dst_result.Get();
	return src_link.parameter->Link(dst_link.parameter->GetAddress());
}

//----------------------------------------------------------------------------//
// Constructor
//    @param in_parameter - pointer to the parameter that represents a link.
//    @param in_id - id of the linked parameter.
ReadSharedPtrLinkTask::ReadSharedPtrLinkTask(const BaseParameter* in_parameter,
                                             size_t in_id) : ReadLinkTask(in_parameter, in_id)
{ }

// Searches for the linked parameter (but in special 'shared' pool) and then
// calls ut::meta::Parameter::Link() method with this parameter as an argument.
//    @param linker - reference to the linker object.
//    @return - error if failed.
Optional<Error> ReadSharedPtrLinkTask::Execute(Linker& linker)
{
	// search for the parameter that is being linked
	Optional<Link&> src_result = linker.FindLinkByParameter(parameter);
	if (!src_result)
	{
		// add log event
		String error_desc = "There is no associated link for this parameter: ";
		error_desc += Print(static_cast<const void*>(parameter));

		// exit
		return Error(error::not_found, error_desc);
	}

	// search for the linked parameter
	Optional<InputSharedCacheElement&> find_result = linker.FindSharedLinkById(destination_id);
	if (!find_result)
	{
		// add log event
		String error_desc = "Couldn't find shared linked parameter with id \"";
		error_desc += Print(src_result->id) + "\" by id: " + Print(destination_id);

		// exit
		return Error(error::not_found, error_desc);
	}

	// link parameters
	Link& src_link = src_result.Get();
	InputSharedCacheElement& cached_element = find_result.Get();
	return src_link.parameter->Link(cached_element.ptr->GetAddress());
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
