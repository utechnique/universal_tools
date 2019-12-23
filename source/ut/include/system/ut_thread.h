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
#include "system/ut_sync.h"
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
// Blocks the execution of the current thread for at least the specified @ms
//    @param ms - milliseconds to wait
void Sleep(uint32 ms);

//----------------------------------------------------------------------------//
// Returns the id of the current thread
//    @return - id of the current thread
ThreadId GetCurrentThreadId();

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
	void Exit();

	// Pure virtual asynchronous function, you have to implement
	// your realization in derived class. Use @exit_request to understand
	// when you are to stop any work and exit the function
	virtual void Execute() = 0;

protected:
	// Synchronized thred-safe variable to indicate
	// when Execute() must stop it's work and exit
	Synchronized<bool> exit_request;
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
	// Constructor, starts a new thread with ut::Job::Execute() function
	//    @param job - job object, whose Execute() function will be
	//                 called asynchronously in separate thread
	Thread(UniquePtr<Job> job);

	// Destructor, sends exit request to the @job, and wait it's completion,
	// then thread object is destroyed. Use Kill() if you don't want to
	// wait the completion, but consider that's a bad practice to violently
	// kill a thread. Try to normally finish (join) all threads.
	~Thread();

	// Sends exit request to the @job
	void Exit();

	// Violently kills a thread, not waiting for completion
	// use this function only for extreme cases
	void Kill();

private:
	// Starts a new thread, Windows realization uses _beginthreadex() to run
	// a thread, Linux realization uses pthread_create(), Android uses the
	// same as linux, but overrides pthread signals to avoid run-time errors.
	Optional<Error> Start();

	// Entry function for the new thread, calls job_ptr->Execute() internally
	static THREAD_PROCEDURE Entry(Job* job_ptr);

	// Job to be executed
	UniquePtr<Job> job;

	// Thread handle, used only in windows
	ThreadHandle handle;

	// Thread id
	ThreadId id;

	// Thread is valid till this variable is 'true'
	bool active;
};

// Shorter type name for UniquePtr<Thread>
typedef UniquePtr<Thread> ThreadPtr;

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//