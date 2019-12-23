//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "meta/linkage/ut_meta_linker.h"
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
// Adds a new task to the queue. All tasks can be executed calling a
// Linker::Execute() method.
//    @param task - unique pointer to the task.
//    @return - ut::Error if failed.
Optional<Error> Linker::AddTask(UniquePtr<LinkTask> task)
{
	if (!tasks.Add(Move(task)))
	{
		return Error(error::out_of_memory);
	}
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Executes all tasks. Must be called after all parameters have been cached.
//    @return - ut::Error if failed.
Optional<Error> Linker::Execute()
{
	// execute all write tasks
	for (size_t i = 0; i < tasks.GetNum(); i++)
	{
		Optional<Error> execute_error = tasks[i]->Execute(*this);
		if (execute_error)
		{
			return execute_error;
		}
	}

	// clean task pool
	tasks.Empty();

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Adds unique shared object to the output cache. Call it to register a shared
// object while serializing parent entity.
//    @param ptr - shared pointer to the holder of the SharedPtr object.
//    @param address - address of the shared object.
//    @return - ut::Error if failed.
Optional<Error> Linker::CacheOutputSharedObject(const SharedPtr<SharedPtrHolderBase>& ptr,
                                                const void* address)
{
	// check if this object already exists
	for (size_t i = 0; i < output_shared_cache.GetNum(); i++)
	{
		if (output_shared_cache[i].address == address)
		{
			return Optional<Error>(); // e x i t
		}
	}

	// add to the cache
	OutputSharedCacheElement cache_element = { ptr, address };
	if (!output_shared_cache.Add(Move(cache_element)))
	{
		return Error(error::out_of_memory);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Adds unique shared object to the input cache. Call it to add a shared
// object that is already deserialized and is ready to be linked with.
//    @param ptr - shared pointer to the holder of the SharedPtr object.
//    @param id - identifier associated with the shared object.
//    @return - ut::Error if failed.
Optional<Error> Linker::CacheInputSharedObject(const SharedPtr<SharedPtrHolderBase>& ptr,
                                               size_t id)
{
	// check if this object already exists
	for (size_t i = 0; i < input_shared_cache.GetNum(); i++)
	{
		if (input_shared_cache[i].id == id)
		{
			return Optional<Error>(); // e x i t
		}
	}

	// add to the cache
	InputSharedCacheElement cache_element = { ptr, id };
	if (!input_shared_cache.Add(Move(cache_element)))
	{
		return Error(error::out_of_memory);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Adds unique shared object to the registry. Call it to register a shared
// object while deserializing parent entity.
//    @param ptr - shared pointer to the holder of the SharedPtr object.
//    @param id - identifier associated with the shared object.
//    @return - ut::Error if failed.
Optional<Error> Linker::RegisterInputSharedObject(const SharedPtr<SharedPtrHolderBase>& ptr,
                                                  size_t id)
{
	// check if this object already exists
	for (size_t i = 0; i < preliminary_shared_cache.GetNum(); i++)
	{
		if (preliminary_shared_cache[i].id == id)
		{
			return Optional<Error>(); // e x i t
		}
	}

	// check final cache too
	for (size_t i = 0; i < input_shared_cache.GetNum(); i++)
	{
		if (input_shared_cache[i].id == id)
		{
			return Optional<Error>(); // e x i t
		}
	}

	// add to the cache
	InputSharedCacheElement cache_element = { ptr, id };
	if (!preliminary_shared_cache.Add(Move(cache_element)))
	{
		return Error(error::out_of_memory);
	}

	// success
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Returns an array of shared parameters that are ready for serialization.
Array<OutputSharedCacheElement> Linker::MoveOutputSharedCache()
{
	Array<OutputSharedCacheElement> cache(Move(output_shared_cache));
	output_shared_cache.Empty();
	return cache;
}

//----------------------------------------------------------------------------->
// Returns array of registered shared parameters, that were registered
// during serialization, but must be loaded separately to be able to be
// inked with.
Array<InputSharedCacheElement> Linker::MovePreliminarySharedCache()
{
	Array<InputSharedCacheElement> cache(Move(preliminary_shared_cache));
	preliminary_shared_cache.Empty();
	return cache;
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

//----------------------------------------------------------------------------->
// Searches for a cached link using a pointer to the managed object.
//    @param address - address of the object that is managed by desired parameter.
//    @return - reference to the cached link.
Optional< Ref<Link> > Linker::FindLinkByAddress(const void* address)
{
	for (size_t i = 0; i < links.GetNum(); i++)
	{
		if (links[i]->parameter->GetAddress() == address)
		{
			return Ref<Link>(links[i].GetRef());
		}
	}

	// nothing was found
	return Optional< Ref<Link> >();
}

//----------------------------------------------------------------------------->
// Searches for a cached link using it's identifier.
//    @param id - id of the desired parameter.
//    @return - reference to the cached link.
Optional< Ref<Link> > Linker::FindLinkById(size_t id)
{
	for (size_t i = 0; i < links.GetNum(); i++)
	{
		if (links[i]->id == id)
		{
			return Ref<Link>(links[i].GetRef());
		}
	}

	// nothing was found
	return Optional< Ref<Link> >();
}

//----------------------------------------------------------------------------->
// Searches for a cached shared link using it's identifier.
//    @param id - id of the desired parameter.
//    @return - reference to the cached element.
Optional< Ref<InputSharedCacheElement> > Linker::FindSharedLinkById(size_t id)
{
	// find linked parameter by id
	for (size_t i = 0; i < input_shared_cache.GetNum(); i++)
	{
		// check if identifiers match
		if (input_shared_cache[i].id == id)
		{
			return Ref<InputSharedCacheElement>(input_shared_cache[i]);
		}
	}

	// nothing was found
	return Optional< Ref<InputSharedCacheElement> >();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//