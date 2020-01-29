//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_base_parameter.h"
#include "pointers/ut_shared_ptr.h"
#include "meta/ut_meta_controller.h"
#include "meta/linkage/ut_meta_link.h"
#include "meta/linkage/ut_meta_shared_holder.h"
#include "meta/linkage/ut_meta_link_task.h"
#include "meta/linkage/ut_meta_link_cache.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Linker is a class that helps to save/load linked parameters.
// Entities like smart pointers (or even raw pointers) can't be easily
// serialized/deserialized using straightforward methods because managed
// address value can't be used in direct form. Linker collects info about
// parameters during serialization and links them afterwards.
class Linker
{
public:
	// Constructor, initializes id generator
	Linker();

	// Generates unique id.
	//    @return - unique id.
	size_t GenerateId();

	// Adds a new link to the cache. Every parameter must be added to the linker
	// during serialization/deserialization. Even if this parameter is not a link.
	//    @param parameter - shared pointer to the parameter to be added.
	//    @param id - unique id of the provided parameter.
	//    @return - ut::Error if failed.
	Optional<Error> AddLink(const SharedPtr<BaseParameter>& parameter, size_t id);

	// Adds a new task to the queue. All tasks can be executed calling a
	// Linker::Execute() method.
	//    @param task - unique pointer to the task.
	//    @return - ut::Error if failed.
	Optional<Error> AddTask(UniquePtr<LinkTask> task);

	// Executes all tasks. Must be called after all parameters have been cached.
	//    @return - ut::Error if failed.
	Optional<Error> Execute();

	// Adds unique shared object to the output cache. Call it to register a shared
	// object while serializing parent entity.
	//    @param ptr - shared pointer to the holder of the SharedPtr object.
	//    @param address - address of the shared object.
	//    @return - ut::Error if failed.
	Optional<Error> CacheOutputSharedObject(const SharedPtr<SharedPtrHolderBase>& ptr,
	                                        const void* address);

	// Adds unique shared object to the input cache. Call it to add a shared
	// object that is already deserialized and is ready to be linked with.
	//    @param ptr - shared pointer to the holder of the SharedPtr object.
	//    @param id - identifier associated with the shared object.
	//    @return - ut::Error if failed.
	Optional<Error> CacheInputSharedObject(const SharedPtr<SharedPtrHolderBase>& ptr,
	                                       size_t id);

	// Adds unique shared object to the registry. Call it to register a shared
	// object while deserializing parent entity.
	//    @param ptr - shared pointer to the holder of the SharedPtr object.
	//    @param id - identifier associated with the shared object.
	//    @return - ut::Error if failed.
	Optional<Error> RegisterInputSharedObject(const SharedPtr<SharedPtrHolderBase>& ptr,
	                                          size_t id);

	// Returns an array of shared parameters that are ready for serialization.
	Array<OutputSharedCacheElement> MoveOutputSharedCache();

	// Returns array of registered shared parameters, that were registered
	// during serialization, but must be loaded separately to be able to be
	// inked with.
	Array<InputSharedCacheElement> MovePreliminarySharedCache();

	// Writes id using a provided state of meta controller.
	//    @param state - state of the meta controller where the id must be
	//                   written.
	//    @param id - id value to be written.
	//    @return - ut::Error if failed.
	Optional<Error> WriteLinkId(Controller state, size_t id) const;

	// Searches for a cached link using a pointer to the desired parameter.
	//    @param parameter - address of the parameter to be found in cache.
	//    @return - reference to the cached link.
	Optional<Link&> FindLinkByParameter(const BaseParameter* parameter);

	// Searches for a cached link using a pointer to the managed object.
	//    @param address - address of the object that is managed by desired parameter.
	//    @return - reference to the cached link.
	Optional<Link&> FindLinkByAddress(const void* address);

	// Searches for a cached link using it's identifier.
	//    @param id - id of the desired parameter.
	//    @return - reference to the cached link.
	Optional<Link&> FindLinkById(size_t id);

	// Searches for a cached shared link using it's identifier.
	//    @param id - id of the desired parameter.
	//    @return - reference to the cached element.
	Optional<InputSharedCacheElement&> FindSharedLinkById(size_t id);

private:
	// Helps to generate unique identifiers, holds the last generated id.
	size_t id_generator;

	// Cached parameters.
	Array< UniquePtr<Link> > links;

	// Tasks.
	Array< UniquePtr<LinkTask> > tasks;

	// Shared objects to be written in the end of the serialization process.
	Array<OutputSharedCacheElement> output_shared_cache;

	// Shared objects that are ready to be linked with during
	// deserialization process.
	Array<InputSharedCacheElement> input_shared_cache;

	// Shared objects that were registered during serialization, but they must
	// be loaded separately to be able to be linked with.
	Array<InputSharedCacheElement> preliminary_shared_cache;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//