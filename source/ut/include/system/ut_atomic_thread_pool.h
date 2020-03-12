//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "system/ut_thread_pool.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::AtomicPoolJob is a template class representing a job for each thread
// in a thread pool. Template argument is a return type of a thread task.
template<typename ReturnType>
class AtomicPoolJob : public ut::Job
{
	// Type of the unique pointer to the managed task.
	typedef UniquePtr< BaseTask<void> > UniqueTaskPtr;
public:
	// Constructor.
	AtomicPoolJob() : busy(0)
	{}

	// Waits for a task and then executes it.
	void Execute()
	{
		while (!exit_request.Read())
		{
			if (atomics::interlocked::Read(&busy) == 2)
			{
				task->Execute();
				atomics::interlocked::Store(&busy, 0);
			}
			this_thread::Yield();
		}
	}

	// Assigns a new task if thread is in idle state.
	//    @param new_task - reference to the unique pointer to the task
	//                      to be executed.
	//    @return - 'true' if task was accepted or 'false' if thread is busy.
	bool SetTask(UniqueTaskPtr& new_task)
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
	UniqueTaskPtr task;
	int32 busy;
};

//----------------------------------------------------------------------------//
// Specialized ut::Scheduler template version where synchronization is
// performed using atomic operations.
template<typename ReturnType, typename Combiner>
class Scheduler<ReturnType, Combiner, pool_sync::atomic>
{
	// Only corresponding thread pool can create a scheduler, therefore it must
	// have access to the scheduler's constructor.
	template <typename, ut::pool_sync::Method> friend class ThreadPool;

	// Type of the unique pointer to the managed task.
	typedef UniquePtr< BaseTask<ReturnType> > UniqueTaskPtr;

public:
	// Assigns a task to a thread from the thread pool. If all threads are busy,
	// waits until one of them ends current task.
	//    @param task - unique pointer to the task to be executed.
	void Enqueue(UniqueTaskPtr task)
	{
		atomics::interlocked::Increment(&counter);
		auto function = MemberFunction<Scheduler, void(UniqueTaskPtr)>(this, &Scheduler::ExecuteTask);
		pool.Enqueue(new Task<void(UniqueTaskPtr)>(function, Move(task)));
	}

	// Waits until all tasks finish.
	//    @return - reference to the combiner.
	Combiner& WaitForCompletion()
	{
		while (atomics::interlocked::Read(&counter) != 0)
		{
			this_thread::Yield();
		}
		return combiner;
	}

private:
	// Executes provided task and decreases counter of tasks by one.
	//    @param task - unique pointer to the task to be executed.
	void ExecuteTask(UniqueTaskPtr task)
	{
		ThreadCombinerHelper<ReturnType, Combiner>::Combine(combiner, task.GetRef(), lock);
		atomics::interlocked::Decrement(&counter);
	}

	// Constructor.
	//    @param thread_pool - pool that owns the scheduler.
	Scheduler(ThreadPool<ReturnType, pool_sync::atomic>& thread_pool) : pool(thread_pool), counter(0)
	{}

	//  pool that owns the scheduler
	ThreadPool<ReturnType, pool_sync::atomic>& pool;

	// counter of the active tasks
	int counter;

	// combiner that combines results
	Combiner combiner;

	// lock for the combiner
	Mutex lock;
};

//----------------------------------------------------------------------------//
// Specialized ut::ThreadPool template version where synchronization is
// performed using atomic operations.
template<typename ReturnType>
class ThreadPool<ReturnType, pool_sync::atomic>
{
	// Job type for all threads in a pool.
	typedef AtomicPoolJob<ReturnType> JobType;

	// Type of the task to be executed in a thread.
	typedef UniquePtr< BaseTask<void> > UniqueTaskPtr;

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
	Scheduler<ReturnType, Combiner, pool_sync::atomic> CreateScheduler()
	{
		return Scheduler<ReturnType, Combiner, pool_sync::atomic>(*this);
	}

	// Creates a scheduler with default combiner.
	Scheduler<ReturnType, DefaultCombiner<ReturnType>, pool_sync::atomic> CreateScheduler()
	{
		return Scheduler<ReturnType, DefaultCombiner<ReturnType>, pool_sync::atomic>(*this);
	}

	// Waits for a free thread and assigns provided task to it.
	//    @param task - task to be executed in a thread.
	void Enqueue(UniqueTaskPtr task)
	{
		while (true)
		{
			bool thread_belongs_to_pool = false;
			const ut::ThreadId current_thread_id = this_thread::GetId();
			for (size_t i = 0; i < size; i++)
			{
				// check if current thread is one of the threads in a pool
				if (threads[i]->GetId() == current_thread_id)
				{
					thread_belongs_to_pool = true;
					continue;
				}

				// attempt to enqueue a task
				JobType& job = static_cast<JobType&>(threads[i]->GetJobRef());
				if (job.SetTask(Move(task)))
				{
					return; // thread was free and accepted a task
				}
			}

			// check if task must be executed in the current thread
			if (thread_belongs_to_pool)
			{
				task->Execute(); // task wasn't enqueued, but current
				return;          // thread is one of the threads in a pool
			}

			// task wasn't enqueued, and we must try again
			this_thread::Yield();
		}
	}

private:
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