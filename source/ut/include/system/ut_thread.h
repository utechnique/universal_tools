//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_array.h"
#include "pointers/ut_unique_ptr.h"
#include "system/ut_mutex.h"
#include "system/ut_lock.h"
#include "system/ut_atomic.h"
#include "templates/ut_task.h"
//----------------------------------------------------------------------------//
#if UT_WINDOWS
// 'Yield()' macro of WinBase.h spoils ut::this_thread::Yield() function's name.
#undef Yield
// 'GetJob()' macro of winspool.h spoils ut::Thread::GetJob() function's name.
#undef GetJob
#endif
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Platform-specific thread types, currently you should define
//    @type ThreadId - integer representing the id of the thread
//    @type ThreadHandle - thread handle, used only in Windows now
//    @define THREAD_PROCEDURE - return type of the thread procedure
#if UT_WINDOWS
	typedef uint32 ThreadId;
	typedef HANDLE ThreadHandle;
	#define THREAD_PROCEDURE unsigned int __stdcall 
#elif UT_UNIX
	typedef pthread_t ThreadId;
	typedef pthread_t ThreadHandle;
	#define THREAD_PROCEDURE void*
#else
	#error Thread types are not implemented
#endif

//----------------------------------------------------------------------------//
// ut::this_thread namespace groups a set of functions that access the
// current thread.
namespace this_thread
{
	// Blocks the execution of the current thread for at least the specified @ms
	//    @param ms - milliseconds to wait
	void Sleep(uint32 ms);

	// Provides a hint to the implementation to reschedule the execution of
	// threads, allowing other threads to run.
	void Yield();

	// Returns the id of the current thread
	//    @return - id of the current thread
	ThreadId GetId();
}

//----------------------------------------------------------------------------//
// Returns the number of processors
//    @return - number of processors
uint32 GetNumberOfProcessors();

//----------------------------------------------------------------------------//
// ut::Job is an atomic piece of work, used in ut::Thread
// ut::Job::Execute() is performed asynchronously, you have to implement
// derived class and define your realization of Execute() function
class Job : public NonCopyable
{
public:
	// Constructor, @exit_request is set to 'false' by default
	Job();

	// Destructor is virtual, ut::Job is polymorphic type
	virtual ~Job();

	// Safely sets exit_request to 'true', can be called from another thread
	virtual void Exit();

	// Pure virtual asynchronous function, you have to implement
	// your version in derived class. Use @exit_request to understand
	// when you are to stop any work and exit the function
	virtual void Execute() = 0;

protected:
	// Synchronized thred-safe variable to indicate
	// when Execute() must stop it's work and exit
	Atomic<bool> exit_request;
};

//----------------------------------------------------------------------------//
// ut::Thread represents a single thread of execution. Threads allow multiple
// functions to execute concurrently. Threads begin execution immediately upon
// construction of the associated thread (pending any OS scheduling delays),
// starting at the top-level ut::Job::Execute() function of the ut::Job object
// that is provided as a constructor argument @job.
class Thread : public NonCopyable
{
public:
	// Constructor, launches provided function in a new thread.
	//    @param proc - function to be called from a new thread.
	Thread(Function<void()> proc);

	// Constructor, starts a new thread with ut::Job::Execute() function
	//    @param job - job object, whose Execute() function will be
	//                 called asynchronously in separate thread
	explicit Thread(UniquePtr<Job> job);

	// Constructor, launches provided task in a new thread.
	//    @param proc - task to be executed in a new thread.
	explicit Thread(UniquePtr< BaseTask<void> > proc);

	// Destructor, sends exit request to the @job, and wait it's completion,
	// then thread object is destroyed. Use Kill() if you don't want to
	// wait the completion, but consider that's a bad practice to violently
	// kill a thread. Try to normally finish (join) all threads.
	~Thread();

	// Returns id of the thread.
	ThreadId GetId() const;

	// Returns reference to the current job.
	Optional<const Job&> GetJob() const;

	// Returns reference to the current job.
	Optional<Job&> GetJob();

	// Sends exit request to the @job.
	void Exit();

	// Blocks the current thread until the thread identified by *this
	// finishes its execution.
	void Join();

	// Violently kills a thread, not waiting for completion
	// use this function only for extreme cases.
	void Kill();

private:
	// Entry function for the new thread, calls @task->Execute() internally.
	static THREAD_PROCEDURE Entry(BaseTask<void>* proc);

	// Starts a new thread using @task member, windows variant uses
	// _beginthreadex() to run a thread, Linux variant uses pthread_create().
	Optional<Error> Start();

	// job to be executed (optional)
	UniquePtr<Job> job;

	// procedure
	UniquePtr< BaseTask<void> > task;

	// Thread handle, used only in windows
	ThreadHandle handle;

	// Thread id
	ThreadId id;

	// Thread is valid till this variable is 'true'
	bool active;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//