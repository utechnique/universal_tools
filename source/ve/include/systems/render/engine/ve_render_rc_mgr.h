//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "systems/render/engine/ve_render_resource.h"
#include "systems/render/engine/ve_render_cfg.h"
#include "systems/render/engine/ve_render_image_loader.h"
#include "systems/render/engine/ve_render_frame.h"
#include "systems/render/engine/resources/ve_render_mesh.h"
#include "systems/render/engine/resources/ve_render_map.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ResourceManager is a helper class to conveniently operate with
// render resources.
class ResourceManager
{
	// AcquisitionGuard is a safety mechanism to ensure only one resource with
	// the same name is being loaded simultaneously, all other threads except
	// the very first one must wait until a desired resource will be loaded.
	struct AcquisitionGuard
	{
		ut::Mutex mutex;
		ut::ConditionVariable result_ready_cvar;
		ut::ConditionVariable waiters_ended_cvar;
		ut::Optional< ut::Optional<ut::Error> > result;
		ut::uint32 waiter_count = 0;
	};

	typedef ut::HashMap<ut::String, ut::SharedPtr<AcquisitionGuard> > AcquisitionApplicants;

public:
	// Constructor.
	ResourceManager(Device& device_ref, const Config<Settings>& cfg);

	// Updates buffer contents with provided data. Can be used as a convenient
	// wrapper around MapBuffer + UnmapBuffer functions. Note that buffer must be
	// created with memory usage flag that provides appropriate kind of cpu access.
	//    @param context - reference to the ve::render::Context object to perform
	//                     perform update operation.
	//    @param buffer - reference to the ve::render::Buffer object to be updated.
	//    @param data - reference to the ve::render::Buffer object to be updated.
	ut::Optional<ut::Error> UpdateBuffer(Context& context,
	                                     Buffer& buffer,
	                                     const void* data);

	// Finds a resource by name.
	template<typename ResourceType>
	ut::Result<RcRef<ResourceType>, ut::Error> Find(const ut::String& name)
	{
		// lock resources
		ut::ScopeRWLock scope_lock(lock, ut::RWLock::Access::read);

		// find resource id by name
		ut::Optional<Resource::Id&> id = names.Find(name);
		if (!id)
		{
			return ut::MakeError(ut::error::not_found);
		}

		// find a resource by it's id
		ut::Optional<ReferencedResource&> rc = resources.Find(id.Get());
		UT_ASSERT(rc);

		// check types
		const ut::DynamicType& dynamic_type = rc->ptr->Identify();
		if (dynamic_type.GetHandle() != ut::GetPolymorphicHandle<ResourceType>())
		{
			return ut::MakeError(ut::error::types_not_match);
		}

		// success
		return RcRef<ResourceType>(static_cast<ResourceType&>(rc->ptr.GetRef()), rc->ref_counter);
	}

	// Generates a resource.
	template<typename ResourceType>
	inline ut::Result<RcRef<ResourceType>, ut::Error> Acquire(const ut::String& name)
	{
		// final result will be stored here
		ut::Result<RcRef<ResourceType>, ut::Error> result;

		// a guard to protect the desired resource from being constructed
		// simultaneously from different threads
		ut::SharedPtr<AcquisitionGuard> acquisition_guard;

		// figure out if this function call is the first to acquire the desired
		// resource in a concurrent run
		bool is_first_to_acquire;
		{
			ut::ScopeSyncLock<AcquisitionApplicants> acquisition_scope_lock(acquisition_lock);
			ut::Optional<ut::SharedPtr<AcquisitionGuard>&> active_guard = acquisition_scope_lock.Get().Find(name);
			is_first_to_acquire = !active_guard.HasValue();
			if (is_first_to_acquire)
			{
				acquisition_guard = ut::MakeShared<AcquisitionGuard>();
				acquisition_scope_lock.Get().Insert(name, acquisition_guard);
			}
			else
			{
				acquisition_guard = active_guard.Get();

				// apply self as a waiter incrementing the counter
				ut::ScopeLock guard_lock(acquisition_guard->mutex);
				acquisition_guard->waiter_count++;
			}
		}

		// if this thread is the first who acquired the desired resource,
		// then this thread is responsible to construct the resource and
		// inform all other waiting threads about the result
		if (is_first_to_acquire)
		{
			// check if the resource already exist
			result = Find<ResourceType>(name);

			// try to construct the resource if wasn't loaded yet
			if (!result && result.GetAlt().GetCode() == ut::error::not_found)
			{
				result = creator.Create<ResourceType>(name);
			}

			// store the result in the guard object and wait for all threads to
			// receive this result
			ut::ScopeSyncLock<AcquisitionApplicants> acquisition_scope_lock(acquisition_lock);
			ut::ScopeLock guard_lock(acquisition_guard->mutex);
			acquisition_guard->result = result ? ut::Optional<ut::Error>() :
				                        ut::Optional<ut::Error>(result.GetAlt());
			acquisition_guard->result_ready_cvar.WakeAll();

			while (acquisition_guard->waiter_count > 0)
			{
				acquisition_guard->waiters_ended_cvar.Wait(guard_lock);
			}

			acquisition_scope_lock.Get().Remove(name);
		}
		else
		{
			// if another thread already started the acquisition process, this
			// thread must just wait for the result that will be stored in an
			// acquisition guard
			ut::ScopeLock guard_lock(acquisition_guard->mutex);
			
			while (!acquisition_guard->result.HasValue())
			{
				acquisition_guard->result_ready_cvar.Wait(guard_lock);
			}

			const ut::Optional<ut::Error>& failed_result = acquisition_guard->result.Get();
			if (failed_result)
			{
				result = ut::MakeError(failed_result.Get());
			}
			else
			{
				result = Find<ResourceType>(name);
			}
			
			acquisition_guard->waiter_count--;

			// the last waiting thread to receive the result is responsible to
			// wake main acquisition thread to inform that there is no more
			// waiters
			if (acquisition_guard->waiter_count == 0)
			{
				acquisition_guard->waiters_ended_cvar.WakeOne();
			}
		}

		return result;
	}

	// Enqueues a deletion of the desired resource.
	//    @param id - unique identifier of the resource to be deleted.
	void DeleteResource(Resource::Id id);

	// Processes internal resource events, such as deletion, etc.
	void Update();

	// Takes ownership of provided resource.
	template<typename ResourceType>
	RcRef<ResourceType> AddResource(ResourceType resource,
	                                ut::Optional<ut::String> name = ut::Optional<ut::String>())
	{
		// lock resources
		ut::ScopeRWLock scope_lock(lock, ut::RWLock::Access::write);

		// wrap resource into unique pointer to the base class
		ut::UniquePtr<Resource> rc_unique_ptr = ut::MakeUnique<ResourceType>(ut::Move(resource));

		// generate unique id for the resource
		Resource::Id id = id_generator.Generate();

		// update the name/id map
		if (name)
		{
			ut::Optional<ut::Pair<const ut::String,
			                      Resource::Id>&> insert_name_result = names.Insert(name.Get(), id);
			UT_ASSERT(!insert_name_result);
			if (insert_name_result)
			{
				ut::String error_desc = ut::String("Unable to add a resource with name \"") +
					name.Get() + "\", the resource with such name already exists.";
				throw ut::Error(ut::error::already_exists, ut::Move(error_desc));
			}
		}

		// create unique resource object with a reference counter
		ReferencedResource unique_rc(*this, ut::Move(rc_unique_ptr), id, ut::Move(name));

		// create reference
		RcRef<ResourceType> ref(static_cast<ResourceType&>(unique_rc.ptr.GetRef()),
		                        unique_rc.ref_counter);

		// add to the map
		ut::Optional<ut::Pair<const Resource::Id,
		                      ReferencedResource>&> insert_rc_result = resources.Insert(id, ut::Move(unique_rc));
		UT_ASSERT(!insert_rc_result);
		return ref;
	}

	// Returns the creator of the desired resource type.
	template<typename ResourceType>
	ResourceCreator<ResourceType>& GetCreator()
	{
		return creator.Get< ResourceCreator<ResourceType> >();
	}

	// a mesh representing a fullscreen quad, 2 triangles, 6 vertices
	RcRef<Mesh> fullscreen_quad;

	// images
	RcRef<Map> img_black;
	RcRef<Map> img_white;
	RcRef<Map> img_red;
	RcRef<Map> img_green;
	RcRef<Map> img_blue;
	RcRef<Map> img_normal;
	RcRef<Map> img_checker;

private:
	// Creates internal engine resources (primitives, common 1x1 textures, etc.)
	ut::Optional<ut::Error> CreateEngineResources();

	// Generates unique resource id.
	IdGenerator<Resource::Id> id_generator;

	// Read-write lock providing thread safety for resource modification.
	ut::RWLock lock;

	// Read-write lock providing thread safety for resource acquisition.
	ut::Synchronized<AcquisitionApplicants> acquisition_lock;

	// Managed resources.
	ut::HashMap<Resource::Id, ReferencedResource> resources;
	ut::HashMap<ut::String, Resource::Id> names;

	// Resources enqueued for deletion.
	ut::Array< ut::Array< ut::UniquePtr<Resource> > > garbage;

	// Id of the current frame, this value is needed to track lifetime
	// of the resource. For example a resource can't be deleted immediately,
	// manager must ensure that this resource isn't used by gpu, thus it will
	// be deleted only after a full cycle of frames in flight.
	ut::uint32 frame_counter;

	// Reference to the render device.
	Device& device;

	// Number of frames in flight.
	const ut::uint32 frames_in_flight;

	// Resource generator.
	ResourceCreatorCollection<Map, Mesh> creator;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
