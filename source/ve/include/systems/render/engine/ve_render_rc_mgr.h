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

	// Creates a mesh representing a 2d rectangle.
	ut::Result<RcRef<Mesh>, ut::Error> CreateRect(const ut::Vector<2>& position,
	                                              const ut::Vector<2>& extent);

	// Creates a mesh representing a box.
	ut::Result<RcRef<Mesh>, ut::Error> CreateBox(const ut::Vector<3>& position,
	                                             const ut::Vector<3>& extent,
	                                             ut::Optional<ut::String> name);

	// Creates an image filled with solid color.
	ut::Result<RcRef<Map>, ut::Error> CreateImage(ut::uint32 width,
	                                              ut::uint32 height,
	                                              const ut::Color<4, ut::byte>& color,
	                                              ut::Optional<ut::String> name,
	                                              bool is_cubemap = false);

	// Finds a resource by name.
	template<typename ResourceType>
	ut::Result<RcRef<ResourceType>, ut::Error> Find(const ut::String& name)
	{
		// lock resources
		ut::ScopeRWLock scope_lock(lock, ut::access_read);

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

	// Enqueues a deletion of the desired resource.
	//    @param id - unique identifier of the resource to be deleted.
	void DeleteResource(Resource::Id id);

	// Processes internal resource events, such as deletion, etc.
	void Update();

	// Takes ownership of provided resource.
	template<typename ResourceType>
	RcRef<ResourceType> AddResource(ResourceType resource, ut::Optional<ut::String> name = ut::Optional<ut::String>())
	{
		// lock resources
		ut::ScopeRWLock scope_lock(lock, ut::access_write);

		// wrap resource into unique pointer to the base class
		ut::UniquePtr<Resource> rc_unique_ptr = ut::MakeUnique<ResourceType>(ut::Move(resource));

		// generate unique id for the resource
		Resource::Id id = id_generator.Generate();

		// update the name/id map
		if (name)
		{
			bool insert_name_result = names.Insert(name.Get(), id);
			UT_ASSERT(insert_name_result);
		}

		// create unique resource object with a reference counter
		ReferencedResource unique_rc(*this, ut::Move(rc_unique_ptr), id, ut::Move(name));

		// create reference
		RcRef<ResourceType> ref(static_cast<ResourceType&>(unique_rc.ptr.GetRef()), unique_rc.ref_counter);

		// add to the map
		bool insert_result = resources.Insert(id, ut::Move(unique_rc));
		UT_ASSERT(insert_result);
		return ref;
	}

	// a mesh representing a fullscreen quad, 2 triangles, 6 vertices
	RcRef<Mesh> fullscreen_quad;

	// primitives
	RcRef<Mesh> cube;

	// images
	RcRef<Map> img_black;
	RcRef<Map> img_white;
	RcRef<Map> img_red;
	RcRef<Map> img_green;
	RcRef<Map> img_blue;
	RcRef<Map> img_normal;

private:
	// Creates a default material (white, roughness is 1, albedo is 0)
	ut::Result<Material, ut::Error> CreateDefaultMaterial();

	// Creates internal engine resources (primitives, common 1x1 textures, etc.)
	ut::Optional<ut::Error> CreateEngineResources();

	// Generates unique resource id.
	IdGenerator<Resource::Id> id_generator;

	// Read-write lock providing thread safety.
	ut::RWLock lock;

	// Managed resources.
	ut::AVLTree<Resource::Id, ReferencedResource> resources;
	ut::AVLTree<ut::String, Resource::Id> names;

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
