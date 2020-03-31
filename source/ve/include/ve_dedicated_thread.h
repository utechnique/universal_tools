//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::BaseDedicatedThread is a base class implementing core threading features
// while ve::DedicatedThread is inherited from it and can be specialized for
// different types of object.
template<typename ObjectType>
class BaseDedicatedThread
{
public:
	typedef ut::Function<void(ObjectType&)> DedicatedTask;

	// Constructor, creates a new thread, then desired object is created.
	// Constructor exits only after the object is fully initialized.
	template<typename... Args>
	BaseDedicatedThread(Args... args) : stop(false), object_ptr(nullptr)
	{
		// create thread
		auto thread_proc = ut::MemberFunction<BaseDedicatedThread, void(Args...)>(this, &BaseDedicatedThread::ThreadProcedure<Args...>);
		ut::UniquePtr< ut::BaseTask<void> > thread_task = ut::MakeUnique< ut::Task<void(Args...)> >(ut::Move(thread_proc), ut::Forward<Args>(args)...);
		thread = ut::MakeUnique<ut::Thread>(ut::Move(thread_task));
	}

	// Executes provided task in a thread associated with the managed object.
	void Enqueue(DedicatedTask task)
	{
		// if this function was called inside a managed thread
		// then just execute the task
		if (thread->GetId() == ut::this_thread::GetId())
		{
			task(*object_ptr);
			return;
		}

		// only one thread can call this function at once
		ut::ScopeLock queue_lock(queue_mutex);

		{ // wait for current task to be picked up before supplying a new one
			ut::ScopeLock lock(task_mutex);
			while (current_task && !stop)
			{
				task_idle.Wait(lock);
			}

			// exit if thread is closed
			if (stop)
			{
				return;
			}

			// set a new task that will be grabbed by a worker thread
			current_task = Move(task);
		}

		// notify thread that a new task is ready to be processed
		task_idle.WakeOne();

		// wait for a task to complete
		ut::ScopeLock lock(task_mutex);
		while (current_task && !stop)
		{
			task_idle.Wait(lock);
		}
	}

	// Destructor. Notifies and joins dedicated thread.
	~BaseDedicatedThread()
	{
		{ // inform all waiters before closing
			ut::ScopeLock lock(task_mutex);
			stop = true;
			task_idle.WakeAll();
		}

		// wait for the dedicated thread to finish
		thread->Join();
	}

private:
	// Thread procedure for the managed object.
	// Object is created here passing provided arguments to constructor.
	template<typename... Args>
	void ThreadProcedure(Args... args)
	{
		try
		{
			// create object
			ObjectType object(ut::Forward<Args>(args)...);

			// process tasks
			while (ProcessTask(object));
		}
		catch (const ut::Error& error)
		{
			ut::log << error.GetDesc() << ut::cret;
			exit(0);
		}
	}

	// Takes task from the queue and processes it in the current thread.
	//    @param object - reference to the managed object.
	//    @return - 'true' if ok, 'false' if thread is about to exit.
	bool ProcessTask(ObjectType& object)
	{
		{ // wait until a task is provided
			ut::ScopeLock lock(task_mutex);
			while (!stop && !current_task)
			{
				task_idle.Wait(lock);
			}

			// check if thread is closed
			if (stop)
			{
				return false;
			}

			// execute current task
			DedicatedTask& task = current_task.Get();
			task(object);
			current_task.Empty();
		}

		// wake Enqueue() function if it's waiting
		task_idle.WakeOne();

		// success
		return true;
	}

	// dedicated thread
	ut::UniquePtr<ut::Thread> thread;
	
	// indicates if object was initialized
	bool ready;

	// synchronization objects for task
	ut::Mutex queue_mutex;
	ut::Mutex task_mutex;
	ut::ConditionVariable task_idle;

	// pointer to the managed object
	ObjectType* object_ptr;

	// bool variable indicating when to stop thread
	bool stop;

	// task that is waiting to be processed by managed object
	ut::Optional<DedicatedTask> current_task;
};

//----------------------------------------------------------------------------//
// ve::DedicatedThread is a template class providing convenient way to create
// any object in a separate thread and to interact with it in a safe manner.
template<typename ObjectType>
class DedicatedThread : public BaseDedicatedThread<ObjectType>
{
	typedef BaseDedicatedThread<ObjectType> Base;
public:
	template<typename... Args>
	DedicatedThread(Args... args) : Base(ut::Forward<Args>(args)...)
	{}
};

//----------------------------------------------------------------------------//
// Specialized version of the ve::DedicatedThread template for unique pointers.
template<typename ObjType>
class DedicatedThread< ut::UniquePtr<ObjType> > : public BaseDedicatedThread< ut::UniquePtr<ObjType> >
{
	typedef BaseDedicatedThread< ut::UniquePtr<ObjType> > Base;
	typedef ut::Function<void(ObjType&)> WrappedTask;
public:
	// Constructor accepts only one argument - unique pointer.
	DedicatedThread(ut::UniquePtr<ObjType> ptr) : Base(ut::Move(ptr))
	{}

	// Make a wrapper around Enqueue() method of the base class
	void Enqueue(WrappedTask task)
	{
		typename Base::DedicatedTask wrap([&](ut::UniquePtr<ObjType>& p) { task(p.GetRef()); });
		Base::Enqueue(ut::Move(wrap));
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//