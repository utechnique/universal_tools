//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_base_parameter.h"
#include "pointers/ut_shared_ptr.h"
#include "meta/ut_meta_controller.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Link is a structure containing a pointer to the parameter which
// can be subsequently linked with another parameter during saving/loading
// a meta snapshot. Also it has unique id that helps to identify this parameter
// during deserialization.
struct Link
{
	SharedPtr<BaseParameter> parameter;
	size_t parameter_id;
};

//----------------------------------------------------------------------------//
// ut::meta::Linker is a class that helps to save/load linked parameters.
// Entities like smart pointers (or even raw pointers) can't be easily
// serialized/deserialized using straightforward methods because managed
// address value can't be used in direct form. Linker collects info about
// parameters during serialization and links them afterwards.
class Linker
{
private:
	// ut::meta::Linker::WriteTask is a structure that helps to serialize links.
	// Linker connects parameters after serialization, therefore parameter that
	// is linked with another parameter must create a special task, so that id
	// of that linked parameter could be written afterwards.
	struct WriteTask
	{
		Link& link;
		Controller state;
		const void* linked_address;
	};

	// ut::meta::Linker::ReadTask is a structure that helps to deserialize links.
	// Linker connects parameters after serialization, therefore parameter that
	// is linked with another parameter must create a special task, so that linker
	// could find that linked parameter by id.
	struct ReadTask
	{
		Link& link;
		size_t destination_id;
	};

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

	// Adds write task to the buffer. This task will be executed during a call to 
	// Linker::Execute() function.
	//    @param parameter - address of the parameter that contains a link.
	//    @param state - saved state of the meta controller, it is used to write
	//                   correct id in a correct place inside a binary stream or a text node.
	//    @param linked_address - address of the linked object.
	//    @return - ut::Error if failed.
	Optional<Error> CreateWriteTask(const BaseParameter* parameter,
	                                Controller state,
	                                const void* linked_address);

	// Adds read task to the buffer. This task will be executed during a call to 
	// Linker::Execute() function.
	//    @param parameter - address of the parameter that contains a link.
	//    @param id - id of the linked parameter.
	//    @return - ut::Error if failed.
	Optional<Error> CreateReadTask(const BaseParameter* parameter, size_t id);

	// Executes all tasks. Must be called after all parameters have been cached.
	//    @return - ut::Error if failed.
	Optional<Error> Execute();
	
private:
	// Searches for the linked parameter by memory address. Then writes it's id
	// to a special place inside a stream or text node using captured state of
	// meta controller.
	//    @parameter task - reference to the task to be executed.
	//    @return - ut::Error if failed.
	Optional<Error> ExecuteWriteTask(const WriteTask& task);

	// Searches for the linked parameter by id. Then links this parameter
	// by a call to the ut::meta::BaseParameter::Link() virtual function.
	//    @parameter task - reference to the task to be executed.
	//    @return - ut::Error if failed.
	Optional<Error> ExecuteReadTask(const ReadTask& task);

	// Writes id using a provided state of meta controller.
	//    @param state - state of the meta controller where the id must be
	//                   written.
	//    @param id - id value to be written.
	//    @return - ut::Error if failed.
	Optional<Error> WriteLinkId(Controller state, size_t id) const;

	// Searches for a cached link using a pointer to the desired parameter.
	//    @param parameter - address of the parameter to be found in cache.
	//    @return - reference to the cached link.
	Optional< Ref<Link> > FindLinkByParameter(const BaseParameter* parameter);

private:
	// Helps to generate unique identifiers, holds the last generated id.
	size_t id_generator;

	// Cached parameters.
	Array< UniquePtr<Link> > links;

	// Tasks.
	Array<WriteTask> write_tasks;
	Array<ReadTask> read_tasks;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//