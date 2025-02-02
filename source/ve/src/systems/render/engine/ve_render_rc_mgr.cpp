//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_rc_mgr.h"
#include "systems/render/engine/ve_render_frame.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
ResourceManager::ResourceManager(Device& device_ref,
                                 const Config<Settings>& cfg) : device(device_ref)
                                                              , frame_counter(0)
                                                              , frames_in_flight(cfg.frames_in_flight)
                                                              , garbage(cfg.frames_in_flight)
                                                              , creator(device, *this)
{
	ut::Optional<ut::Error> engine_rc_error = CreateEngineResources();
	if (engine_rc_error)
	{
		throw ut::Error(engine_rc_error.Move());
	}
}

//----------------------------------------------------------------------------->
// Updates buffer contents with provided data. Can be used as a convenient
// wrapper around MapBuffer + UnmapBuffer functions. Note that buffer must be
// created with memory usage flag that provides appropriate kind of cpu access.
//    @param context - reference to the ve::render::Context object to perform
//                     perform update operation.
//    @param buffer - reference to the ve::render::Buffer object to be updated.
//    @param data - reference to the ve::render::Buffer object to be updated.
ut::Optional<ut::Error> ResourceManager::UpdateBuffer(Context& context,
                                                      Buffer& buffer,
                                                      const void* data)
{
	ut::Result<void*, ut::Error> map_result = context.MapBuffer(buffer, ut::access_write);
	if (!map_result)
	{
		return map_result.MoveAlt();
	}

	ut::memory::Copy(map_result.Get(), data, buffer.GetInfo().size);

	context.UnmapBuffer(buffer);

	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------->
// Enqueues a deletion of the desired resource.
//    @param id - unique identifier of the resource to be deleted.
void ResourceManager::DeleteResource(Resource::Id id)
{
	// lock resources
	ut::ScopeRWLock scope_lock(lock, ut::access_write);

	// find desired resource by id
	ut::Optional<ReferencedResource&> resource = resources.Find(id);
	if (!resource)
	{
		return;
	}

	// remove name entry
	if (resource->name)
	{
		names.Remove(resource->name.Get());
	}

	// move the resource to the deletion queue, also note that
	// the resource will be deleted only after a full cycle of frames,
	// this guarantees that resource will not be used by gpu
	if (!garbage[frame_counter].Add(ut::Move(resource->ptr)))
	{
		throw ut::Error(ut::error::out_of_memory);
	}

	// remove empty pointer
	resources.Remove(id);

	// release id
	id_generator.Release(id);
}

//----------------------------------------------------------------------------->
// Processes internal resource events, such as deletion, etc.
void ResourceManager::Update()
{
	// increment frame counter
	if (++frame_counter >= frames_in_flight)
	{
		frame_counter = 0;
	}
	ut::Array< ut::UniquePtr<Resource> >& frame_garbage = garbage[frame_counter];

	// resources can have tree-like ownership that's why we need
	// to delete resources one by one to prevent dead lock
	while (true)
	{
		// resource will be stored in this pointer and will be destroyed
		// in a destructor at the end of this loop's iteration
		ut::UniquePtr<Resource> rc_to_destroy;

		{ // lock the resources only to take one resource out of garbage container
			ut::ScopeRWLock scope_lock(lock, ut::access_write);

			// if all resources scheduled for deletion ended,
			// then we can break the loop
			if (frame_garbage.IsEmpty())
			{
				break;
			}

			// get the last resource of the garbage container out
			const size_t last_id = frame_garbage.Count() - 1;
			rc_to_destroy = ut::Move(frame_garbage[last_id]);
			frame_garbage.Remove(last_id);
		}
	}
}

//----------------------------------------------------------------------------->
// Creates internal engine resources (primitives, common 1x1 textures, etc.)
ut::Optional<ut::Error> ResourceManager::CreateEngineResources()
{
	ResourceCreator<Map>& map_creator = GetCreator<Map>();
	ResourceCreator<Mesh>& mesh_creator = GetCreator<Mesh>();

	// 1x1 black image
	ut::Result<RcRef<Map>, ut::Error> map = map_creator.CreateFromSolidColor(ut::Color<4, ut::byte>(0,0,0,255));
	if (!map)
	{
		return map.MoveAlt();
	}
	img_black = map.Move();

	// 1x1 white image
	map = map_creator.CreateFromSolidColor(ut::Color<4, ut::byte>(255, 255, 255, 255));
	if (!map)
	{
		return map.MoveAlt();
	}
	img_white = map.Move();

	// 1x1 red image
	map = map_creator.CreateFromSolidColor(ut::Color<4, ut::byte>(255, 0, 0, 255));
	if (!map)
	{
		return map.MoveAlt();
	}
	img_red = map.Move();

	// 1x1 green image
	map = map_creator.CreateFromSolidColor(ut::Color<4, ut::byte>(0, 255, 0, 255));
	if (!map)
	{
		return map.MoveAlt();
	}
	img_green = map.Move();

	// 1x1 blue image
	map = map_creator.CreateFromSolidColor(ut::Color<4, ut::byte>(0, 0, 255, 255));
	if (!map)
	{
		return map.MoveAlt();
	}
	img_blue = map.Move();

	// 1x1 normal map
	map = map_creator.CreateFromSolidColor(ut::Color<4, ut::byte>(127, 127, 255, 255));
	if (!map)
	{
		return map.MoveAlt();
	}
	img_normal = map.Move();

	// fullscreen rect
	ut::Result<RcRef<Mesh>, ut::Error> mesh = mesh_creator.CreateEyeSpaceRect();
	if (!mesh)
	{
		return mesh.MoveAlt();
	}
	fullscreen_quad = mesh.Move();

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//