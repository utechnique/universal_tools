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
// Forward declarations.
class ResourceManager;
class Device;

//----------------------------------------------------------------------------//
// ve::render::Resource is a class encapsulating gpu data that can be created,
// modified or deleted only from the render thread by it's resource manager.
// Such resources are very delicate and have very special lifecycle due to
// complexity of the render pipeline. Do not try to modify it outside the render
// thread, the only valid way to use it outside of the rendering engine - create
// an empty ve::render::RcRef reference to the resource in a render unit and wait
// till render engine initializes it using appropriate unit policy template.
class Resource : public ut::Polymorphic
{
public:
	// Some resources having ve::render::ResourceCreator template specialized
	// for their type can be initialized procedurally passing a specialy
	// crafted name called a 'prompt' to the ve::render::ResourceCreator::Create()
	// function. For example "box|w:2|h:10|d:4" creates a mesh of a box with
	// width=2, height=10 and depth=4. First comes the resource type name
	// followed with attributes (separated with the special symbol '|')
	// describing the desired resource.
	// ve::render::Resource::GeneratorPrompt encapsulates rules for
	// procedural generation of resources.
	struct GeneratorPrompt
	{
		// Every prompt must start with this character.
		static const char skStarterChr = '*';

		// Null-terminated version of @skStarterChr.
		static const char skStarter[2];

		// Attributes are separated by these characters.
		static const char skSeparatorChr0 = '|';
		static const char skSeparatorChr1 = '?';
		static const char skSeparatorChr2 = '\"';

		// This character separates attribute's type from its value.
		static const char skValueSeparatorChr = ':';

		// ve::render::Resource::GeneratorPrompt::Attribute represents a
		// specific quality of the desired resource in the form of
		// type/value pair.
		struct Attribute
		{
			char type;
			ut::String value;
		};
		typedef ut::Array<Attribute> Attributes;

		// Checks if the provided text is a generator prompt.
		//    @param text - a string to be tested, it must start
		//                  with @skStarter to pass the check.
		//    @return - the result of the check.
		static bool Check(const ut::String& text);

		// Parses the provided generator prompt into separate attributes.
		//    @param prompt - generator prompt.
		//    @param out_attributes - an array of attributes to store
		//                            parsed attributes in.
		//    @return - a name of the resource's type or ut::Error if failed.
		static ut::Result<ut::String, ut::Error> Parse(const ut::String& prompt,
		                                               Attributes& out_attributes);
	};

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

	// All resources must be dynamically identifiable. Don't forget to register
	// resource classes via UT_REGISTER_TYPE macro.
	virtual const ut::DynamicType& Identify() const = 0;
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
		// Counter constructor, takes a reference to the resource manager that
		// is used to delete the resource when its reference count becomes zero.
		Counter(ResourceManager& rc_mgr,
		        Resource::Id rc_id);

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
	// Constructor, takes an ownership of the provided resource and
	// initializes a counter of references to this resource.
	ReferencedResource(ResourceManager& rc_mgr,
	                   ut::UniquePtr<Resource> unique_rc,
	                   Resource::Id rc_id,
	                   ut::Optional<ut::String> rc_name = ut::Optional<ut::String>());

private:

	// Managed resource.
	ut::UniquePtr<Resource> ptr;

	// Reference counter.
	ut::SharedPtr<Counter> ref_counter;

	// Unique name of the resource.
	ut::Optional<ut::String> name;
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
// ve::render::ResourceCreator is a template class to be specialized for every
// resource type supporting the generation from prompt. Each such specialized
// version must have a function Create() with signature
// ut::Result<RcRef<ResourceType>, ut::Error>(const ut::String&).
template<typename ResourceType>
class ResourceCreator
{
public:
	ResourceCreator(Device& device, ResourceManager& rc_mgr)
	{}

	ut::Result<RcRef<ResourceType>, ut::Error> Create(const ut::String& name)
	{
		return ut::MakeError(ut::error::not_implemented);
	}
};

// ve::render::ResourceCreatorCollection is a convenient template class to
// contain ve::render::ResourceCreator objects for desired resource types. Just
// pass desired resource types as template arguments and call Create() template
// method to create a resource of appropriate type.
template<typename... ResourceTypes>
class ResourceCreatorCollection : public ut::Tuple<ResourceCreator<ResourceTypes>...>
{
    typedef ut::Tuple<ResourceCreator<ResourceTypes>...> Base;
public:
	ResourceCreatorCollection(Device& device, ResourceManager& rc_mgr) :
		Base(ResourceCreator<ResourceTypes>(device, rc_mgr)...)
	{}

	template<typename ResourceType>
	ut::Result<RcRef<ResourceType>, ut::Error> Create(const ut::String& name)
	{
		return Base::template Get< ResourceCreator<ResourceType> >().Create(name);
	}
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
