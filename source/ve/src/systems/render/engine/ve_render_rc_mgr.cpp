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
                                 const Config<Settings>& cfg) noexcept : device(device_ref)
                                                                       , frame_counter(0)
                                                                       , frames_in_flight(cfg.frames_in_flight)
                                                                       , garbage(cfg.frames_in_flight)
{}

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

// Creates vertex buffer representing a fullscreen quad.
ut::Result<Buffer, ut::Error> ResourceManager::CreateFullscreenQuad()
{
	// create screen space quad
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::vertex;
	buffer_info.usage = render::memory::gpu_immutable;
	buffer_info.size = Frame::QuadVertex::size * 6;
	buffer_info.stride = Frame::QuadVertex::size;
	buffer_info.data.Resize(buffer_info.size);

	Frame::QuadVertex::Type* vertices = reinterpret_cast<Frame::QuadVertex::Type*>(buffer_info.data.GetAddress());
	vertices[0].position = ut::Vector<2>(-1, 1);
	vertices[0].texcoord = ut::Vector<2>(0, 0);
	vertices[1].position = ut::Vector<2>(1, 1);
	vertices[1].texcoord = ut::Vector<2>(1, 0);
	vertices[2].position = ut::Vector<2>(-1, -1);
	vertices[2].texcoord = ut::Vector<2>(0, 1);

	vertices[3] = vertices[1];
	vertices[4] = vertices[2];
	vertices[5].position = ut::Vector<2>(1, -1);
	vertices[5].texcoord = ut::Vector<2>(1, 1);

	return device.CreateBuffer(ut::Move(buffer_info));
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
	// lock resources
	ut::ScopeRWLock scope_lock(lock, ut::access_write);

	// increment frame counter
	if (++frame_counter >= frames_in_flight)
	{
		frame_counter = 0;
	}

	// delete unused resources
	garbage[frame_counter].Empty();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//