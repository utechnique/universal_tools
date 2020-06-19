//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "system/ut_sync.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Constructor.
SyncPoint::Request::Request(WeakPtr<SyncPoint::OccupantId> in_occupant_thread) : active(false)
                                                                               , thread_id(this_thread::GetId())
                                                                               , occupant_thread(ut::Move(in_occupant_thread))
{}

// Move constructor.
SyncPoint::Request::Request(SyncPoint::Request&&) noexcept = default;

// Waits for the synchronization with ut::SyncPoint::Synchronize().
void SyncPoint::Request::Synchronize()
{
	ScopeLock scope_lock(mutex);
	while (!active)
	{
		cvar.Wait(scope_lock);
	}
}

// Informs issuer that synchronization started and waits for the request
// to call ut::SyncPoint::Request::Finish().
void SyncPoint::Request::StartAndWaitCompletion(Mutex& occupant_mutex)
{
	{ // inform issuer that synchronization started
		ScopeLock scope_lock(mutex);
		active = true;

		// cache id of the current thread to prevent dead lock
		// in recursive AcquireLock() function calls
		ScopeLock occupant_lock(occupant_mutex);
		SharedPtr<OccupantId> pinned_occupant_thread = occupant_thread.Pin();
		if (pinned_occupant_thread)
		{
			pinned_occupant_thread.GetRef() = thread_id;
		}
	}
	cvar.WakeOne();

	// wait for issuer to finish synchronization
	ScopeLock scope_lock(mutex);
	while (active)
	{
		cvar.Wait(scope_lock);
	}
}

// Informs synchronization point that synchronization is over.
void SyncPoint::Request::Finish()
{
	{ // inform synchronization point that job is done
		ScopeLock scope_lock(mutex);
		active = false;

		// reset occupant thread id
		SharedPtr<OccupantId> pinned_occupant_thread = occupant_thread.Pin();
		if (pinned_occupant_thread)
		{
			pinned_occupant_thread.GetRef() = Optional<ThreadId>();
		}
	}
	cvar.WakeOne();
}

//----------------------------------------------------------------------------//
// Constructor.
SyncPoint::Lock::Lock(WeakPtr<SyncPoint::Request> in_request) : request(in_request)
{}

// Move constructor.
SyncPoint::Lock::Lock(Lock&&) noexcept = default;

// Destructor.
SyncPoint::Lock::~Lock()
{
	Release();
}

// Finishes synchronization process.
void SyncPoint::Lock::Release()
{
	SharedPtr<Request> shared_request = request.Pin();
	if (!shared_request)
	{
		return;
	}
	shared_request->Finish();
	request.Reset();
}

//----------------------------------------------------------------------------//
// Constructor.
SyncPoint::SyncPoint() : occupant_thread(MakeShared<OccupantId>())
{}

// Move constructor.
SyncPoint::SyncPoint(SyncPoint&&) noexcept = default;

// Synchronizes all pending requests. This function is thread-safe.
void SyncPoint::Synchronize()
{
	// move pending requests to the temp buffer
	Array< SharedPtr<Request> >& locked_requests = requests.Lock();
	Array< SharedPtr<Request> > temp_buffer(Move(locked_requests));
	requests.Unlock();

	// kick off pending requests
	const size_t request_count = temp_buffer.GetNum();
	for (size_t i = 0; i < request_count; i++)
	{
		temp_buffer[i]->StartAndWaitCompletion(occupant_mutex);
	}
}

// Waits for the synchronization with ut::SyncPoint::Synchronize()
// and returns a lock controlling synchronization time.
SyncPoint::Lock SyncPoint::AcquireLock()
{
	// don't synchronize if Synchronize() function is already busy
	// with a request from current thread (recursion)
	occupant_mutex.Lock();
	if (occupant_thread.GetRef() && occupant_thread->Get() == this_thread::GetId())
	{
		occupant_mutex.Unlock();
		return Lock(SharedPtr<Request>());
	}

	// create new request
	SharedPtr<Request> request = MakeShared<Request>(occupant_thread);

	// unlock occupant thread id mutex
	occupant_mutex.Unlock();

	// add a new request
	Array< SharedPtr<Request> >& locked_requests = requests.Lock();
	locked_requests.Add(request);
	requests.Unlock();

	// wait for the synchronization
	request->Synchronize();

	// return a lock to issuer
	return Lock(request);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//