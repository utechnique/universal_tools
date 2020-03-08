//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "system/ut_thread.h"
#include "containers/ut_array.h"
#include "pointers/ut_unique_ptr.h"
#include "templates/ut_task.h"
#include "templates/ut_default_combiner.h"
#include "system/ut_condition_variable.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// UT provides different methods to synchronize worker threads in a pool.
namespace pool_sync
{
	enum Method
	{
		// synchronization via condition variables, common case
		cond_var,

		// synchronization via atomic operations, loads cpu to the max,
		// but might be faster for a busy pool
		atomic
	};

	// default synchronization method
	static const Method skDefaultMethod = cond_var;
}

//----------------------------------------------------------------------------//
// ut::ThreadPoolJob is a template class representing a job for each thread
// in a thread pool. Template argument is a return type of a thread task.
template<typename ReturnType, pool_sync::Method sync_method = pool_sync::skDefaultMethod>
class ThreadPoolJob;

// ut::Scheduler is a template class that syncronizes execution of the tasks
// in a thread pool and combines the result of this execution.
template<typename ReturnType,
         typename Combiner = DefaultCombiner<ReturnType>,
         pool_sync::Method sync_method = pool_sync::skDefaultMethod>
class Scheduler;

// ut::ThreadPool is a template class that holds a custom number of active
// threads and can enqueue tasks to a free thread thus executing different tasks
// simultaneously. Template argument is a return type of a task.
template<typename ReturnType, pool_sync::Method sync_method = pool_sync::skDefaultMethod>
class ThreadPool;

//----------------------------------------------------------------------------//
// ut::ThreadCombinerHelper is a template class helping to apply combiner to the
// result of the executed task.
template<typename ReturnType, typename Combiner>
struct ThreadCombinerHelper
{
	static inline void Combine(Combiner& combiner, BaseTask<ReturnType>& task, Mutex& mutex)
	{
		ReturnType result(task.Execute());
		ScopeLock lock(mutex);
		combiner(Move(result));
	}
};

// Specialized version of the ut::ThreadCombinerHelper for 'void' type
// doesn't lock mutex and doesn't apply a combiner, because there is nothing
// to combine.
template<typename Combiner>
struct ThreadCombinerHelper<void, Combiner>
{
	static inline void Combine(Combiner& combiner, BaseTask<void>& task, Mutex& mutex)
	{
		task.Execute();
	}
};

//----------------------------------------------------------------------------//
// Specialized ut::ThreadPoolJob template version where synchronization is
// performed using condition variables.
template<typename ReturnType>
class ThreadPoolJob<ReturnType, pool_sync::cond_var> : public ut::Job
{
	// Type of the thread pool.
	typedef ThreadPool<ReturnType, pool_sync::cond_var> PoolType;

public:
	// Constructor.
	ThreadPoolJob(PoolType& in_pool) : pool(in_pool)
	{}

	// Waits for a task and then executes it.
	void Execute()
	{
		while (pool.DispatchTask(true))
		{ }
	}

private:
	PoolType& pool;
};

//----------------------------------------------------------------------------//
// Specialized ut::Scheduler template version where synchronization is
// performed using condition variables.
template<typename ReturnType, typename Combiner>
class Scheduler<ReturnType, Combiner, pool_sync::cond_var>
{
	// Only corresponding thread pool can create a scheduler, therefore it must
	// have access to the scheduler's constructor.
	template <typename, ut::pool_sync::Method> friend class ThreadPool;

	// Type of the unique pointer to the managed task.
	typedef UniquePtr< BaseTask<ReturnType> > TaskPtr;

public:
	// Assigns a task to a thread from the thread pool. If all threads are busy,
	// waits until one of them ends current task.
	//    @param task - unique pointer to the task to be executed.
	void Enqueue(TaskPtr task)
	{
		// create and send a task to the pool
		MemberInvoker<void(Scheduler::*)(TaskPtr)> invoker(&Scheduler::ExecuteTask, this);
		Task<void(TaskPtr)> task_wrapper(invoker, Move(task));
		pool.Enqueue(Move(task_wrapper));

		// increment task counter
		ScopeLock lock(mutex);
		counter++;
	}

	// Waits until all tasks finish.
	//    @return - reference to the combiner.
	Combiner& WaitForCompletion()
	{
		// if WaitForCompletion() call was made inside one of the worker
		// threads, then tasks must be processed here in order not to block
		// current worker thread
		if (pool.InWorkerThread())
		{
			while (counter != 0 && pool.DispatchTask(false))
			{ }
		}
		else // otherwise - just wait until all tasks are processed
		{
			ScopeLock lock(mutex);
			while (counter != 0)
			{
				cvar.Wait(lock);
			}
		}
		return combiner;
	}

private:
	// Executes provided task and decreases counter of tasks by one.
	//    @param task - unique pointer to the task to be executed.
	void ExecuteTask(TaskPtr task)
	{
		// combine the result of execution
		ThreadCombinerHelper<ReturnType, Combiner>::Combine(combiner, task.GetRef(), combiner_lock);

		{ // decrement counter after the task was processed 
			ScopeLock lock(mutex);
			counter--;
		}

		// notify scheduler if it waits in WaitForCompletion() function
		cvar.WakeOne();
	}

	// Constructor.
	//    @param thread_pool - pool that owns the scheduler.
	Scheduler(ThreadPool<ReturnType, pool_sync::cond_var>& thread_pool) : pool(thread_pool)
	                                                                    , counter(0)
	{}

	// pool that owns the scheduler
	ThreadPool<ReturnType, pool_sync::cond_var>& pool;

	// counter of the active tasks
	int counter;

	// combiner that combines results
	Combiner combiner;
	Mutex combiner_lock;

	// mutex and condition variable for synchronization
	Mutex mutex;
	ConditionVariable cvar;
};

//----------------------------------------------------------------------------//
// Specialized ut::ThreadPool template version where synchronization is
// performed using condition variables.
template<typename ReturnType>
class ThreadPool<ReturnType, pool_sync::cond_var> : NonCopyable
{
	// Job type for all threads in a pool.
	typedef ThreadPoolJob<ReturnType, pool_sync::cond_var> JobType;

	// Type of the task provided by scheduler.
	typedef Task<void(UniquePtr< BaseTask<ReturnType> >)> TaskType;

public:
	// Constructor.
	//    @param num_threads - number of active threads.
	ThreadPool(size_t num_threads = GetNumberOfProcessors()) : size(num_threads)
	                                                         , threads(num_threads)
	                                                         , stop(false)
	{
		for (size_t i = 0; i < size; i++)
		{
			UniquePtr<Job> job(new JobType(*this));
			threads[i] = new Thread(Move(job));
		}
	}

	// Destructor. Notifies and joins worker threads.
	~ThreadPool()
	{
		{ // inform all workers before closing
			ScopeLock lock(mutex);
			stop = true;
			worker_cvar.WakeAll();
		}

		// wait for all workers to finish corresponding thread
		for (size_t i = 0; i < size; i++)
		{
			threads[i]->Join();
		}
	}

	// Creates a scheduler with custom combiner.
	template<typename Combiner>
	Scheduler<ReturnType, Combiner, pool_sync::cond_var> CreateScheduler()
	{
		return Scheduler<ReturnType, Combiner, pool_sync::cond_var>(*this);
	}

	// Creates a scheduler with default combiner.
	Scheduler<ReturnType, DefaultCombiner<ReturnType>, pool_sync::cond_var> CreateScheduler()
	{
		return Scheduler<ReturnType, DefaultCombiner<ReturnType>, pool_sync::cond_var>(*this);
	}

	// Checks if current thread belongs to pool.
	bool InWorkerThread()
	{
		const ut::ThreadId current_thread_id = this_thread::GetId();
		for (size_t i = 0; i < size; i++)
		{
			if (threads[i]->GetId() == current_thread_id)
			{
				return true;
			}
		}
		return false;
	}

	// Takes current task from the pool and processes it in the current thread.
	//    @param wait - boolean variable whether to wait for a new task to occur
	//    @return - 'true' if ok, 'false' if pool is about to exit
	bool DispatchTask(bool wait)
	{
		Optional<TaskType> task;

		{ // get a new task to execute
			ScopeLock lock(mutex);

			// wait until a task was provided
			while (wait && !stop && !current_task)
			{
				worker_cvar.Wait(lock);
			}

			// check the reason of awakening
			if (current_task)
			{
				task = current_task.Move();
				current_task.Empty();
			}
			else if (stop)
			{
				// fail, exiting right now
				return false;
			}
		}

		// execute task
		if (task)
		{
			// wake ut::ThreadPool::Enqueue() if it's waiting
			pool_cvar.WakeOne();

			// execute a task
			task.Get().Execute();
		}

		// success
		return true;
	}

	// Waits for a free thread and assigns provided task to it.
	//    @param task - task to be executed in a worker thread.
	void Enqueue(TaskType task)
	{
		// task that needs to be processed immediately without dispatching to
		// a worker thread, such a case can occur if ut::ThreadPool::Enqueue()
		// function was called within one of the worker threads
		Optional<TaskType> immediate_task;

		{ // wait for current task to be dispatched by a worker thread
			ScopeLock lock(mutex);
			if (current_task)
			{
				// if task from the previous call is still unprocessed, then
				// maybe it's because we are in one of the worker threads
				if (InWorkerThread())
				{
					immediate_task = current_task.Move();
				}
				else // otherwise wait for another thread
				{
					do
					{
						pool_cvar.Wait(lock);
					} while (current_task);
				}
			}

			// set a new task that will be grabbed by a worker thread
			current_task = Move(task);
		}

		// notify one of the worker threads that a 
		// new task is ready to be processed
		worker_cvar.WakeOne();

		// if current thread is one of the worker threads and all other
		// threads are busy - previous task must be processed immediately,
		// and the task provided as argument remains for the next round
		if (immediate_task)
		{
			immediate_task.Get().Execute();
		}
	}

private:
	// array of active threads
	Array< UniquePtr<Thread> > threads;

	// number of active threads
	const size_t size;

	// task that is waiting to be processed by
	// one of the worker threads
	Optional<TaskType> current_task;

	// bool variable indicating when to stop workers
	bool stop;

	// synchronization objects
	Mutex mutex;
	ConditionVariable worker_cvar; // every worker waits for a task
	ConditionVariable pool_cvar; // pool waits for a free worker
};

//----------------------------------------------------------------------------//
// ut::ThreadPool::TaskType contains ut::UniquePtr thus needs to be constructed
// via non-const reference for cpp standards older than c++11.
#if CPP_STANDARD < 2011
// ThreadPool::TaskType
template<typename T>
struct RefConstness< Task<void(UniquePtr< BaseTask<T> >)> >
UT_SET_CONSTNESS(false)
// Optional<ThreadPool::TaskType>
template<typename T>
struct RefConstness< Optional< Task<void(UniquePtr< BaseTask<T> >)> > >
UT_SET_CONSTNESS(false)
#endif // CPP_STANDARD < 2011
//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//