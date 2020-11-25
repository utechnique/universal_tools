//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Froward declarations.
class ResourceManager;

//----------------------------------------------------------------------------//
// ve::render::Resource is a class encapsulating gpu data that can be created,
// modified or deleted only from the render thread by it's resource manager.
// Such resources are very delicate and have very special lifecycle due to
// complexity of the render pipeline. Do not try to modify it outside the render
// thread, the only valid way to use it - create an empty ve::render::RcRef
// reference to the resource in a render unit and wait till render engine
// initializes it using appropriate unit policy template.
class Resource
{
public:
	// Type of the unique resource identifier.
	typedef ut::uint32 Id;

	// Constructor.
	Resource() noexcept = default;

	// Move constructor.
	Resource(Resource&&) noexcept = default;

	// Move operator is prohibited.
	Resource& operator=(Resource&&) = delete;

	// Copying is prohibited.
	Resource(const Resource&) = delete;
	Resource& operator=(const Resource&) = delete;

	// Polymorphic classes must have virtual destructor.
	virtual ~Resource() = default;
};

//----------------------------------------------------------------------------//
// ve::render::ReferencedResource manages ve::render::Resource object and
// counts references to it.
class ReferencedResource
{
	friend class ResourceManager;
	template<class> friend class RcRef;

	// Counts references.
	struct Counter
	{
		// Counter constructor.
		Counter(ResourceManager& rc_mgr, Resource::Id rc_id);

		// Decrements reference count and enqueues
		// a deletion if it becomes zero.
		void Decrement();

		// Increments reference count.
		void Increment();

		// Reference to the manager created this resource.
		ResourceManager& manager;

		// Number of references to this resource.
		ut::Atomic<ut::uint32> count;

		// Unique identifier of this resource.
		Resource::Id id;
	};

public:
	// Constructor.
	ReferencedResource(ut::UniquePtr<Resource> unique_rc,
	                   Resource::Id rc_id,
	                   ResourceManager& rc_mgr);

private:

	// Managed resource.
	ut::UniquePtr<Resource> ptr;

	// Reference counter.
	ut::SharedPtr<Counter> ref_counter;
};


//----------------------------------------------------------------------------//
// ve::render::RcRef is a template class representing a reference to a render
// resource. This is the only correct way to link a resource.
template<class Resource>
class RcRef
{
public:
	// Default constructor. Reference is invalid.
	RcRef() : rc_ptr(nullptr)
	{}

	// Constructor, accepts a reference to the resource.
	RcRef(Resource& resource,
	      ut::SharedPtr<ReferencedResource::Counter>& shared_counter) : rc_ptr(&resource)
	                                                                  , counter(shared_counter)
	{
		if (shared_counter)
		{
			shared_counter->Increment();
		}
	}

	// Move constructor.
	RcRef(RcRef&& other) noexcept : rc_ptr(other.rc_ptr), counter(ut::Move(other.counter))
	{
		other.rc_ptr = nullptr;
	}

	// Move assignment operator.
	RcRef& operator = (RcRef&& other)
	{
		Decrement();
		rc_ptr = other.rc_ptr;
		counter = ut::Move(other.counter);
		other.rc_ptr = nullptr;
		return *this;
	}

	// Copying is prohibited.
	RcRef(const RcRef&) = delete;
	RcRef& operator = (const RcRef&) = delete;

	// Destructor.
	~RcRef()
	{
		Decrement();
	}

	// Overloaded inheritance operator,
	// provides read access to the owned object.
	const Resource* operator -> () const
	{
		UT_ASSERT(rc_ptr != nullptr);
		return static_cast<const Resource*>(rc_ptr);
	}

	// Overloaded inheritance operator,
	// provides access to the owned object.
	Resource* operator -> ()
	{
		UT_ASSERT(rc_ptr != nullptr);
		return static_cast<Resource*>(rc_ptr);
	}

	// Returns const reference to the managed object.
	const Resource& Get() const
	{
		UT_ASSERT(rc_ptr != nullptr);
		return static_cast<const Resource&>(*rc_ptr);
	}

	// Returns a reference to the managed object.
	Resource& Get()
	{
		UT_ASSERT(rc_ptr != nullptr);
		return static_cast<Resource&>(*rc_ptr);
	}

	// Returns true if reference is valid otherwise it can't be used.
	bool IsValid() const
	{
		return rc_ptr != nullptr && counter.IsValid();
	}

private:
	// Decrements resource reference count.
	void Decrement()
	{
		if (rc_ptr == nullptr)
		{
			return;
		}

		ut::SharedPtr<ReferencedResource::Counter> shared_counter = counter.Pin();
		if (!shared_counter)
		{
			return;
		}

		shared_counter->Decrement();
	}

	Resource* rc_ptr;
	ut::WeakPtr<ReferencedResource::Counter> counter;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
