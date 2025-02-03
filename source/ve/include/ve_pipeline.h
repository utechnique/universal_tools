//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::PipelineCombiner combines results of concurrent pipelines into single
// array of environment commands.
struct PipelineCombiner
{
public:
	// Constructor, creates empty array of commands.
	PipelineCombiner();

	// Concatenates provided result to the current array of commands.
	//    @param result - return value of the ve::Pipeline::Execute function.
	void operator()(System::Result result);

	// Returns current array of commands, original array becomes empty.
	System::Result Move();

private:
	System::Result sum;
	ut::Mutex mutex;
};

//----------------------------------------------------------------------------//
// ve::Pipeline is a class that controls systems and regulates order of calls
// to ve::System::Update() during the main loop.
class Pipeline
{
public:
	// Type of a task that is scheduled in a thread pool.
	typedef ut::Task<System::Result(System::Time, ut::ThreadPool<System::Result>&)> PoolTask;

	// Function signature of a task that is scheduled in a thread pool.
	typedef System::Result(TaskSignature)(System::Time, ut::ThreadPool<System::Result>&);

	// Constructor, managed system remains empty.
	Pipeline();

	// Constructor.
	//    @param sys - shared pointer to the managed system.
	Pipeline(ut::SharedPtr<System> sys);

	// Adds child pipeline that will be executed simultaneously with
	// other parallel pipelines during ve::Pipeline::Execute() call.
	//    @param pipeline - pipeline object to be added.
	//    @return - optional ut::Error if failed to add provided pipeline.
	ut::Optional<ut::Error> AddParallel(Pipeline pipeline);

	// Adds child pipeline that will be executed in series with
	// other serial pipelines during ve::Pipeline::Execute() call.
	//    @param pipeline - pipeline object to be added.
	//    @return - optional ut::Error if failed to add provided pipeline.
	ut::Optional<ut::Error> AddSerial(Pipeline pipeline);

	// Updates systems in the following order:
	// 1. Updates own system.
	// 2. Updates systems in parallel children pipelines simultaneously in
	//    different threads.
	// 3. Updates systems in serial children pipelines sequentially one by one.
	//    @param time_step_ms - time step for the current frame in milliseconds.
	//    @param pool - reference to the thread pool to
	//                  process parallel child pipelines simultaneously.
	//    @return - array of commands to be executed by owning environment.
	System::Result Execute(System::Time time_step_ms,
	                       ut::ThreadPool<System::Result>& pool);

	// Iterates all systems and registers a provided entity.
	//    @param id - id of the provided entity.
	//    @param entity - reference to the entity to be registered.
	void RegisterEntity(Entity::Id id, Entity& entity);

	// Iterates all systems and unregisters a provided entity.
	//    @param id - id of the provided entity.
	void UnregisterEntity(Entity::Id id);

	// Synchronizes the returned component map collection with the internal
	// access map so that the both maps shared the same source.
	//    @return - a reference to the component map collection to
	//              synchronize @component_access with.
	ComponentMapCollection<ComponentMap::Access::read_write> SynchronizeComponents();

private:
	// Collects component maps from all internal systems.
	//    @param map_collection -  reference to the component map collection to
	//                             store component maps in.
	void CollectComponentMaps(ComponentMapCollection<ComponentMap::Access::read_write>& map_collection);

	// Recursively synchronizes the provided component map collection with the
	// internal access map so that the both maps shared the same source.
	//    @param source - a reference to the component map collection to
	//                    synchronize @component_access with.
	void SynchronizeComponents(ComponentMapCollection<ComponentMap::Access::read_write>& source);

	// managed system
	ut::SharedPtr<System> system;

	// Provides an access to the components available for the @system.
	SynchronizableComponentAccessGroup component_access;

	// child pipelines to be executed simultaneously
	ut::Array<Pipeline> parallel;

	// child pipelines to be executed sequentially
	ut::Array<Pipeline> series;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//