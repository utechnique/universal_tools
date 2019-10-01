//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/ut_meta_linker.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Constructor, initializes id generator
Linker::Linker() : id_generator(0)
{ }

//----------------------------------------------------------------------------->
// Generates unique id.
//    @return - unique id.
size_t Linker::GenerateId()
{
	return id_generator++;
}

//----------------------------------------------------------------------------->
// Adds a new link to the cache. Every parameter must be added to the linker
// during serialization/deserialization. Even if this parameter is not a link.
//    @param parameter - shared pointer to the parameter to be added.
//    @param id - unique id of the provided parameter.
//    @return - error if failed.
Optional<Error> Linker::AddLink(const SharedPtr<BaseParameter>& parameter, size_t id)
{
	// Initialize link
	Link link = { parameter, id };

	// Add a link. Link is created as unique ptr because it must
	// have a stable address in memory, ut::Array would change this address
	// during reallocation otherwise.
	if (!links.Add(new Link(Move(link))))
	{
		return Error(error::out_of_memory);
	}

	// Success.
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Adds write task to the buffer. This task will be executed in Linker::Execute()
// function.
//    @param parameter - address of the parameter that contains a link.
//    @param state - saved state of the meta controller, it is used to write
//                   correct id in a correct place inside a binary stream or a text node.
//    @param linked_address - address of the linked object.
//    @return - ut::Error if failed.
Optional<Error> Linker::CreateWriteTask(const BaseParameter* parameter,
                                        Controller state,
                                        const void* linked_address)
{
	// find associated link
	Optional< Ref<Link> > link = FindLinkByParameter(parameter);
	if (!link)
	{
		return Error(error::not_found, "There is no associated link for this parameter.");
	}

	// add a task
	WriteTask task = { link.Get(), Move(state), linked_address };
	if (!write_tasks.Add(task))
	{
		return Error(error::out_of_memory);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Adds read task to the buffer. This task will be executed during a call to 
// Linker::Execute() function.
//    @param parameter - address of the parameter that contains a link.
//    @param id - id of the linked parameter.
//    @return - ut::Error if failed.
Optional<Error> Linker::CreateReadTask(const BaseParameter* parameter, size_t id)
{
	// find associated link
	Optional< Ref<Link> > link = FindLinkByParameter(parameter);
	if (!link)
	{
		return Error(error::not_found, "There is no associated link for this parameter.");
	}

	// add a task
	ReadTask task = { link.Get(), id };
	if (!read_tasks.Add(task))
	{
		return Error(error::out_of_memory);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Executes all tasks. Must be called after all parameters have been cached.
//    @return - ut::Error if failed.
Optional<Error> Linker::Execute()
{
	// execute all write tasks
	for (size_t i = 0; i < write_tasks.GetNum(); i++)
	{
		Optional<Error> write_error = ExecuteWriteTask(write_tasks[i]);
		if (write_error)
		{
			return write_error;
		}
	}

	// execute all read tasks
	for (size_t i = 0; i < read_tasks.GetNum(); i++)
	{
		Optional<Error> read_error = ExecuteReadTask(read_tasks[i]);
		if (read_error)
		{
			return read_error;
		}
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Searches for the linked parameter by memory address. Then writes it's id
// to a special place inside a stream or text node using captured state of
// meta controller.
//    @parameter task - reference to the task to be executed.
//    @return - ut::Error if failed.
Optional<Error> Linker::ExecuteWriteTask(const WriteTask& task)
{
	// find linked parameter by address
	for (size_t i = 0; i < links.GetNum(); i++)
	{
		// check if addresses match
		if (links[i]->parameter->GetAddress() != task.linked_address)
		{
			continue;
		}

		// write id
		return WriteLinkId(task.state, links[i]->parameter_id); // e x i t
	}

	// add log event
	String error_desc = "Couldn't find linked parameter with id \"";
	error_desc += Print(task.link.parameter_id) + "\" by address: " + Print(task.linked_address);
	task.state.GetInfo().log_signal(error_desc);

	// couldn't find linked parameter by address
	return Error(error::not_found);
}

//----------------------------------------------------------------------------->
// Searches for the linked parameter by id. Then links this parameter
// by a call to the ut::meta::BaseParameter::Link() virtual function.
//    @parameter task - reference to the task to be executed.
//    @return - ut::Error if failed.
Optional<Error> Linker::ExecuteReadTask(const ReadTask& task)
{
	// find linked parameter by id
	for (size_t i = 0; i < links.GetNum(); i++)
	{
		// check if addresses match
		if (links[i]->parameter_id != task.destination_id)
		{
			continue;
		}

		// link parameters
		return task.link.parameter->Link(links[i]->parameter.GetRef());
	}

	// add log event
	String error_desc = "Couldn't find linked parameter with id \"";
	error_desc += Print(task.link.parameter_id) + "\" by id: " + Print(task.destination_id);

	// couldn't find linked parameter by address
	return Error(error::not_found, error_desc);
}

//----------------------------------------------------------------------------->
// Writes id using a provided state of meta controller.
//    @param state - state of the meta controller where the id must be
//                   written.
//    @param id - id value to be written.
//    @return - ut::Error if failed.
Optional<Error> Linker::WriteLinkId(Controller state, size_t id) const
{
	// create controller
	Controller controller(Move(state));

	// convert id
	Controller::SizeType converted_id = static_cast<Controller::SizeType>(id);

	// save current stream position
	Result<stream::Cursor, Error> stream_position = controller.GetStreamCursor();
	if (!stream_position)
	{
		return stream_position.MoveAlt();
	}

	// synchronize stream so that it's internal cursor pointed to the place
	// where our id value must be written
	Optional<Error> sync_error = controller.Sync();
	if (sync_error)
	{
		return sync_error;
	}

	// write id
	Optional<Error> write_error = controller.WriteValue<Controller::SizeType>(converted_id);
	if (write_error)
	{
		return write_error;
	}

	// set stream position back to the origin value
	return controller.SetCursor(stream_position.GetResult(), true);
}

//----------------------------------------------------------------------------->
// Searches for a cached link using a pointer to the desired parameter.
//    @param parameter - address of the parameter to be found in cache.
//    @return - reference to the cached link.
Optional< Ref<Link> > Linker::FindLinkByParameter(const BaseParameter* parameter)
{
	for (size_t i = 0; i < links.GetNum(); i++)
	{
		if (links[i]->parameter.Get() == parameter)
		{
			return Ref<Link>(links[i].GetRef());
		}
	}

	// nothing was found
	return Optional< Ref<Link> >();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//