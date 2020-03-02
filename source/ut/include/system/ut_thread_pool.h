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
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Forward declaration.
template<typename ReturnType> class ThreadPool;

//----------------------------------------------------------------------------//
// ut::ThreadPoolJob is a template class representing a job for each thread
// in a thread pool. Template argument is a return type of a thread task.
template<typename ReturnType>
class ThreadPoolJob : public ut::Job
{
	// Type of the unique pointer to the managed task.
	typedef UniquePtr< BaseTask<ReturnType> > TaskPtr;
public:
	// Constructor.
	ThreadPoolJob() : busy(0)
	{}

	// Waits for a task and then executes it.
	void Execute()
	{
		while (!exit_request.Read())
		{
			if (atomics::interlocked::Read(&busy) == 2)
			{
				task.Execute();
				atomics::interlocked::Store(&busy, 0);
			}
			Sleep(0);
		}
	}

	// Assigns a new task if thread is in idle state.
	//    @param new_task - reference to the unique pointer to the task
	//                      to be executed.
	//    @return - 'true' if task was accepted or 'false' if thread is busy.
	bool SetTask(Task<void(TaskPtr)>& new_task)
	{
		if (atomics::interlocked::CompareExchange(&busy, 1, 0) == 0)
		{
			task = Move(new_task);
			atomics::interlocked::Store(&busy, 2);
			return true;
		}
		return false;
	}

	// Checks if thread is processing a task in the moment.
	bool IsBusy()
	{
		return atomics::interlocked::Read(&busy) != 0;
	}

private:
	Task<void(TaskPtr)> task;
	int32 busy;
};

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
// ut::Scheduler is a template class that syncronizes execution of the tasks
// in a thread pool and combines the result of this execution.
template< typename ReturnType, typename Combiner = DefaultCombiner<ReturnType> >
class Scheduler
{
	// Only corresponding thread pool can create a scheduler, therefore it must
	// have access to the scheduler's constructor.
	template <typename> friend class ThreadPool;

	// Type of the unique pointer to the managed task.
	typedef UniquePtr< BaseTask<ReturnType> > TaskPtr;

public:
	// Assigns a task to a thread from the thread pool. If all threads are busy,
	// waits until one of them ends current task.
	//    @param task - unique pointer to the task to be executed.
	void Enqueue(TaskPtr task)
	{
		atomics::interlocked::Increment(&counter);
		MemberInvoker<void(Scheduler::*)(TaskPtr)> invoker(&Scheduler::ExecuteTask, this);
		Task<void(TaskPtr)> task_wrapper(invoker, Move(task));
		pool.Enqueue(Move(task_wrapper));
	}

	// Waits until all tasks finish.
	//    @return - reference to the combiner.
	Combiner& WaitForCompletion()
	{
		while (atomics::interlocked::Read(&counter) != 0)
		{
			Sleep(0);
		}
		return combiner;
	}

private:
	// Executes provided task and decreases counter of tasks by one.
	//    @param task - unique pointer to the task to be executed.
	void ExecuteTask(TaskPtr task)
	{
		ThreadCombinerHelper<ReturnType, Combiner>::Combine(combiner, task.GetRef(), lock);
		atomics::interlocked::Decrement(&counter);
	}

	// Constructor.
	//    @param thread_pool - pool that owns the scheduler.
	Scheduler(ThreadPool<ReturnType>& thread_pool) : pool(thread_pool), counter(0)
	{}

	//  pool that owns the scheduler
	ThreadPool<ReturnType>& pool;

	// counter of the active tasks
	int counter;

	// combiner that combines results
	Combiner combiner;

	// lock for the combiner
	Mutex lock;
};

//----------------------------------------------------------------------------//
// ut::ThreadPool is a template class that holds a custom number of active
// threads and can enqueue task to a free thread thus executing different tasks
// simultaneously. Template argument is a return type of a task.
template< typename ReturnType>
class ThreadPool
{
	// A task can be enqueued only via scheduler, therefore scheduler must
	// have access to the ut::ThreadPool::Enqueue() function.
	template <typename, typename> friend class Scheduler;

	// Job type for all threads in a pool.
	typedef ThreadPoolJob<ReturnType> JobType;

public:
	// Constructor.
	//    @param num_threads - number of active threads.
	ThreadPool(size_t num_threads = GetNumberOfProcessors()) : size(num_threads)
	                                                         , threads(num_threads)
	{
		for (size_t i = 0; i < size; i++)
		{
			UniquePtr<Job> job(new JobType);
			threads[i] = new Thread(Move(job));
		}
	}

	// Creates a scheduler with custom combiner.
	template<typename Combiner>
	Scheduler<ReturnType, Combiner> CreateScheduler()
	{
		return Scheduler<ReturnType, Combiner>(*this);
	}

	// Creates a scheduler with default combiner.
	Scheduler<ReturnType> CreateScheduler()
	{
		return Scheduler<ReturnType>(*this);
	}

private:
	// Waits for a free thread and assigns provided task to it.
	//    @param task - task to be executed in a thread.
	void Enqueue(Task<void(UniquePtr< BaseTask<ReturnType> >)> task)
	{
		while (true)
		{
			bool thread_belongs_to_pool = false;
			for (size_t i = 0; i < size; i++)
			{
				// check if current thread is one of the threads in a pool
				if (threads[i]->GetId() == GetCurrentThreadId())
				{
					thread_belongs_to_pool = true;
					continue;
				}

				// attempt to enqueue a task
				JobType& job = static_cast<JobType&>(threads[i]->GetJobRef());
				if (job.SetTask(task))
				{
					return; // thread was free and accepted a task
				}
			}

			// check if task must be executed in the current thread
			if (thread_belongs_to_pool)
			{
				task.Execute(); // task wasn't enqueued, but current
				return;          // thread is one of the threads in a pool
			}

			// task wasn't enqueued, and we must try again
			Sleep(0);
		}
	}

	// array of active threads
	Array< UniquePtr<Thread> > threads;

	// number of active threads
	const size_t size;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//