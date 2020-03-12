//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_thread.h"
#include "system/ut_memory.h"
#include "error/ut_error.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::this_thread namespace groups a set of functions that access the
// current thread.
namespace this_thread
{
	// Blocks the execution of the current thread for at least the specified @ms
	//    @param ms - milliseconds to wait
	void Sleep(uint32 ms)
	{
#if UT_WINDOWS
		::Sleep(ms);
#elif UT_UNIX
		usleep(ms*1e+3);
#else
#error ut::this_thread::Sleep() is not implemented
#endif
	}

	// Provides a hint to the implementation to reschedule the execution of
	// threads, allowing other threads to run.
	void Yield()
	{
#if UT_WINDOWS
		SwitchToThread();
#elif UT_UNIX
		pthread_yield();
#else
#error ut::this_thread::Yield() is not implemented
#endif
	}

	// Returns the id of the current thread
	//    @return - id of the current thread
	ThreadId GetId()
	{
#if UT_WINDOWS
		return ::GetCurrentThreadId();
#elif UT_UNIX
		return pthread_self();
#else
#error ut::this_thread::GetId() is not implemented
#endif
	}
}

//----------------------------------------------------------------------------//
// Returns the number of processors
//    @return - number of processors
uint32 GetNumberOfProcessors()
{
#if UT_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return static_cast<uint32>(sysinfo.dwNumberOfProcessors);
#elif UT_UNIX
	return static_cast<uint32>(sysconf(_SC_NPROCESSORS_ONLN));
#else
	#error ut::GetNumberOfProcessors() is not implemented
#endif
}

//----------------------------------------------------------------------------//
// class ut::Job                                                              //
//----------------------------------------------------------------------------//
// Constructor, @exit_request is set to 'false' by default
Job::Job() : exit_request(false)
{}

//---------------------------------------------------------------------------->
// Destructor is virtual, ut::Job is polymorphic type
Job::~Job()
{}

//----------------------------------------------------------------------------->
// Safely sets exit_request to 'true', can be called from another thread
void Job::Exit()
{
	exit_request.Store(true);
}

//----------------------------------------------------------------------------//
// class ut::Thread                                                           //
//----------------------------------------------------------------------------//
// Constructor, starts a new thread with ut::Job::Execute() function
//    @param job - job object, whose Execute() function will be
//                 called asynchronously in separate thread
Thread::Thread(UniquePtr<Job> job_ptr) : job(Move(job_ptr))
                                       , handle(0)
                                       , id(0)
                                       , active(false)
{
	UT_ASSERT(job);
	task = new Task<void()>(MemberFunction<Job, void()>(job.Get(), &Job::Execute));
	Optional<Error> launch_error = Start();
	if (launch_error)
	{
		throw launch_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Constructor, launches provided function in a new thread.
//    @param proc - function to be called from a new thread.
Thread::Thread(Function<void()> proc) : handle(0)
                                      , id(0)
                                      , active(false)
{
	UT_ASSERT(proc.IsValid());
	task = new Task<void()>(Move(proc));
	Optional<Error> launch_error = Start();
	if (launch_error)
	{
		throw launch_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Constructor, launches provided task in a new thread.
//    @param proc - task to be executed in a new thread.
Thread::Thread(UniquePtr< BaseTask<void> > proc) : task(Move(proc))
                                                 , handle(0)
                                                 , id(0)
                                                 , active(false)
{
	Optional<Error> launch_error = Start();
	if (launch_error)
	{
		throw launch_error.Move();
	}
}

//----------------------------------------------------------------------------->
// Destructor, sends exit request to the @job, and wait it's completion,
// then thread object is destroyed. Use Kill() if you don't want to
// wait the completion, but consider that's a bad practice to violently
// kill a thread. Try to normally finish (join) all threads.
Thread::~Thread()
{
	Join();
}

//----------------------------------------------------------------------------->
// Returns id of the thread.
ThreadId Thread::GetId() const
{
	return id;
}

//----------------------------------------------------------------------------->
// Returns reference to the current job.
const Job& Thread::GetJobRef() const
{
	UT_ASSERT(job);
	return job.GetRef();
}

//----------------------------------------------------------------------------->
// Returns reference to the current job.
Job& Thread::GetJobRef()
{
	UT_ASSERT(job);
	return job.GetRef();
}

//----------------------------------------------------------------------------->
// Sends exit request to the @job
void Thread::Exit()
{
	if (active && job)
	{
		job->Exit();
	}
}

//----------------------------------------------------------------------------->
// Blocks the current thread until the thread identified by *this
// finishes its execution.
void Thread::Join()
{
	if (active)
	{
		// send 'exit' message to job
		if (job)
		{
			job->Exit();
		}

		// wait for thread to exit is needed only if thread object is being
		// destroyed from the separate thread, note that you can destroy thread
		// object from it's own thread only if there is no code after destruction
		if (this_thread::GetId() != id) // btw. GetCurrentThreadId() is very cheap
		{
			#if UT_WINDOWS
				WaitForSingleObject(handle, INFINITE);
			#elif UT_UNIX
				pthread_join(id, NULL);
			#endif
		}

		// close handle for windows, otherwise thread object will be alive
		// up to the end of the program 
		#if UT_WINDOWS
			CloseHandle(handle);
		#endif

		active = false;
	}
}

//----------------------------------------------------------------------------->
// Violently kills a thread, not waiting for completion
// use this function only for extreme cases
void Thread::Kill(void)
{
#if UT_WINDOWS
	TerminateThread(handle, 0);
	CloseHandle(handle);
#elif UT_UNIX
	pthread_cancel(id);
	pthread_join(id, NULL);
#endif
	job.Delete();
	active = false;
}

//----------------------------------------------------------------------------->
// Starts a new thread with a job, windows realization uses _beginthreadex() to run
// a thread, Linux realization uses pthread_create().
inline Optional<Error> Thread::Start()
{
#if UT_WINDOWS
	handle = reinterpret_cast<HANDLE>(
			_beginthreadex(NULL, // security
			               0, // stack size
			               reinterpret_cast<unsigned int(__stdcall*)(void*)>(Entry),
			               task.Get(), // thread argument
			               0, // initialization flags
			               (unsigned int*)&id)); // thread id (address)

	if (handle == NULL)
	{
		return Error(ConvertErrno(errno));
	}
#elif UT_UNIX
	int result = pthread_create(&id,
	                            NULL,
	                            reinterpret_cast<void* (*)(void*)>(Entry),
	                            task.Get());
	if (result != 0)
	{
		return Error(ConvertErrno(result));
	}
#else
	#error ut::Thread constructor is not implemented
#endif
	active = true;
	return Optional<Error>();
}

//----------------------------------------------------------------------------->
// Entry function for the new thread, calls job_ptr->Execute() internally
THREAD_PROCEDURE Thread::Entry(BaseTask<void>* proc)
{
	if (proc != nullptr)
	{
		proc->Execute();
	}
	return NULL;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//