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
	System::Result MoveResult();

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
	typedef ut::Task<System::Result(ut::ThreadPool<System::Result>&)> PoolTask;

	// Function signature of a task that is scheduled in a thread pool.
	typedef System::Result(TaskSignature)(ut::ThreadPool<System::Result>&);

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
	//    @param pool - reference to the thread pool to
	//                  process parallel children simultaneously.
	// 1. Updates own system.
	// 2. Updates systems in parallel children pipelines simultaneously in
	//    different threads.
	// 3. Updates systems in serial children pipelines sequentially one by one.
	//    @return - array of commands to be executed by owning environment.
	System::Result Execute(ut::ThreadPool<System::Result>& pool);

	// Iterates all systems and registers a provided entity.
	//    @param id - id of the provided entity.
	//    @param entity - reference to the entity to be registered.
	void RegisterEntity(Entity::Id id, Entity& entity);

private:
	// managed system
	ut::SharedPtr<System> system;

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