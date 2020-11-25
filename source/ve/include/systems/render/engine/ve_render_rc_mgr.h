//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "systems/render/ve_render_resource.h"
#include "systems/render/engine/ve_render_cfg.h"
#include "systems/render/engine/ve_render_image_loader.h"
#include "systems/render/engine/ve_render_frame.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ResourceManager is a helper class to conveniently operate with
// render resources.
class ResourceManager
{
public:
	// Constructor.
	ResourceManager(Device& device_ref, const Config<Settings>& cfg) noexcept;

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

	// Creates vertex buffer representing a fullscreen quad.
	ut::Result<Buffer, ut::Error> CreateFullscreenQuad();

	// Enqueues a deletion of the desired resource.
	//    @param id - unique identifier of the resource to be deleted.
	void DeleteResource(Resource::Id id);

	// Processes internal resource events, such as deletion, etc.
	void Update();

	// Takes ownership of provided resource.
	template<typename ResourceType>
	RcRef<ResourceType> AddResource(ResourceType resource)
	{
		// lock resources
		ut::ScopeRWLock scope_lock(lock, ut::access_write);

		// wrap resource into unique pointer to the base class
		ut::UniquePtr<Resource> rc_unique_ptr = ut::MakeUnique<ResourceType>(ut::Move(resource));

		// generate unique id for the resource
		Resource::Id id = id_generator.Generate();

		// create unique resource object with a reference counter
		ReferencedResource unique_rc(ut::Move(rc_unique_ptr), id, *this);

		// create reference
		RcRef<ResourceType> ref(static_cast<ResourceType&>(unique_rc.ptr.GetRef()), unique_rc.ref_counter);

		// add to the map
		bool insert_result = resources.Insert(id, ut::Move(unique_rc));
		UT_ASSERT(insert_result);
		return ref;
	}

private:
	// Generates unique resource id.
	IdGenerator<Resource::Id> id_generator;

	// Read-write lock providing thread safety.
	ut::RWLock lock;

	// Managed resources.
	ut::AVLTree<Resource::Id, ReferencedResource> resources;

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
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
