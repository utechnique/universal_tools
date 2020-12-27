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

//----------------------------------------------------------------------------->
// Creates vertex buffer representing a fullscreen quad.
ut::Result<RcRef<Mesh>, ut::Error> ResourceManager::CreateRect(const ut::Vector<2>& position,
                                                               const ut::Vector<2>& extent)
{
	typedef Vertex<float, 2, float, 2> QuadVertex;

	Buffer::Info buffer_info;
	buffer_info.type = Buffer::vertex;
	buffer_info.usage = render::memory::gpu_immutable;
	buffer_info.size = QuadVertex::size * 6;
	buffer_info.stride = QuadVertex::size;
	buffer_info.data.Resize(buffer_info.size);

	QuadVertex::Type* vertices = reinterpret_cast<QuadVertex::Type*>(buffer_info.data.GetAddress());

	vertices[0].position = ut::Vector<2>(position.X() - extent.X(), position.Y() + extent.Y());
	vertices[0].texcoord = ut::Vector<2>(0, 0);
	vertices[1].position = ut::Vector<2>(position.X() + extent.X(), position.Y() + extent.Y());
	vertices[1].texcoord = ut::Vector<2>(1, 0);
	vertices[2].position = ut::Vector<2>(position.X() - extent.X(), position.Y() - extent.Y());
	vertices[2].texcoord = ut::Vector<2>(0, 1);

	vertices[3] = vertices[1];
	vertices[4] = vertices[2];
	vertices[5].position = ut::Vector<2>(position.X() + extent.X(), position.Y() - extent.Y());
	vertices[5].texcoord = ut::Vector<2>(1, 1);

	ut::Result<Buffer, ut::Error> vertex_buffer = device.CreateBuffer(ut::Move(buffer_info));
	if (!vertex_buffer)
	{
		return ut::MakeError(vertex_buffer.MoveAlt());
	}
	
	InputAssemblyState primitive_info;
	primitive_info.topology = primitive::triangle_list;
	primitive_info.stride = QuadVertex::size;
	primitive_info.elements = QuadVertex::CreateLayout();

	return AddResource<Mesh>(Mesh(2, 6,
	                              vertex_buffer.Move(),
	                              ut::Optional<Buffer>(),
	                              index_type_uint32,
	                              ut::Move(primitive_info)));
}

//----------------------------------------------------------------------------->
// Creates a mesh representing a box.
ut::Result<RcRef<Mesh>, ut::Error> ResourceManager::CreateBox(const ut::Vector<3>& position,
                                                              const ut::Vector<3>& extent)
{
	typedef Vertex<float, 3, float, 2, float, 3, float, 3> CubeVertex;

	constexpr ut::uint32 vertex_count = 24;
	constexpr ut::uint32 face_count = 12;

	const float x = extent.X();
	const float y = extent.Y();
	const float z = extent.Z();

	Buffer::Info vertex_buffer_info;
	vertex_buffer_info.type = Buffer::vertex;
	vertex_buffer_info.usage = render::memory::gpu_immutable;
	vertex_buffer_info.size = CubeVertex::size * vertex_count;
	vertex_buffer_info.stride = CubeVertex::size;
	vertex_buffer_info.data.Resize(vertex_buffer_info.size);
	CubeVertex::Type* v = reinterpret_cast<CubeVertex::Type*>(vertex_buffer_info.data.GetAddress());

	v[0].position = ut::Vector<3>(-x, y, -z); v[0].normal = ut::Vector<3>(0, 1, 0); v[0].texcoord = ut::Vector<2>(0, 0);
	v[1].position = ut::Vector<3>( x, y, -z); v[1].normal = ut::Vector<3>(0, 1, 0); v[1].texcoord = ut::Vector<2>(1, 0);
	v[2].position = ut::Vector<3>( x, y,  z); v[2].normal = ut::Vector<3>(0, 1, 0); v[2].texcoord = ut::Vector<2>(1, 1);
	v[3].position = ut::Vector<3>(-x, y,  z); v[3].normal = ut::Vector<3>(0, 1, 0); v[3].texcoord = ut::Vector<2>(0, 1);

	v[4].position = ut::Vector<3>(-x, -y, -z); v[4].normal = ut::Vector<3>(0, -1, 0); v[4].texcoord = ut::Vector<2>(0, 0);
	v[5].position = ut::Vector<3>( x, -y, -z); v[5].normal = ut::Vector<3>(0, -1, 0); v[5].texcoord = ut::Vector<2>(1, 0);
	v[6].position = ut::Vector<3>( x, -y,  z); v[6].normal = ut::Vector<3>(0, -1, 0); v[6].texcoord = ut::Vector<2>(1, 1);
	v[7].position = ut::Vector<3>(-x, -y,  z); v[7].normal = ut::Vector<3>(0, -1, 0); v[7].texcoord = ut::Vector<2>(0, 1);

	v[8].position  = ut::Vector<3>(-x, -y,  z); v[8].normal  = ut::Vector<3>(-1, 0, 0); v[8].texcoord  = ut::Vector<2>(0, 0);
	v[9].position  = ut::Vector<3>(-x, -y, -z); v[9].normal  = ut::Vector<3>(-1, 0, 0); v[9].texcoord  = ut::Vector<2>(1, 0);
	v[10].position = ut::Vector<3>(-x,  y, -z); v[10].normal = ut::Vector<3>(-1, 0, 0); v[10].texcoord = ut::Vector<2>(1, 1);
	v[11].position = ut::Vector<3>(-x,  y,  z); v[11].normal = ut::Vector<3>(-1, 0, 0); v[11].texcoord = ut::Vector<2>(0, 1);

	v[12].position = ut::Vector<3>(x, -y,  z); v[12].normal = ut::Vector<3>(1, 0, 0); v[12].texcoord = ut::Vector<2>(0, 0);
	v[13].position = ut::Vector<3>(x, -y, -z); v[13].normal = ut::Vector<3>(1, 0, 0); v[13].texcoord = ut::Vector<2>(1, 0);
	v[14].position = ut::Vector<3>(x,  y, -z); v[14].normal = ut::Vector<3>(1, 0, 0); v[14].texcoord = ut::Vector<2>(1, 1);
	v[15].position = ut::Vector<3>(x,  y,  z); v[15].normal = ut::Vector<3>(1, 0, 0); v[15].texcoord = ut::Vector<2>(0, 1);

	v[16].position = ut::Vector<3>(-x, -y, -z); v[16].normal = ut::Vector<3>(0, 0, -1); v[16].texcoord = ut::Vector<2>(0, 0);
	v[17].position = ut::Vector<3>( x, -y, -z); v[17].normal = ut::Vector<3>(0, 0, -1); v[17].texcoord = ut::Vector<2>(1, 0);
	v[18].position = ut::Vector<3>( x,  y, -z); v[18].normal = ut::Vector<3>(0, 0, -1); v[18].texcoord = ut::Vector<2>(1, 1);
	v[19].position = ut::Vector<3>(-x,  y, -z); v[19].normal = ut::Vector<3>(0, 0, -1); v[19].texcoord = ut::Vector<2>(0, 1);

	v[20].position = ut::Vector<3>(-x, -y, z); v[20].normal = ut::Vector<3>(0, 0, 1); v[20].texcoord = ut::Vector<2>(0, 0);
	v[21].position = ut::Vector<3>( x, -y, z); v[21].normal = ut::Vector<3>(0, 0, 1); v[21].texcoord = ut::Vector<2>(1, 0);
	v[22].position = ut::Vector<3>( x,  y, z); v[22].normal = ut::Vector<3>(0, 0, 1); v[22].texcoord = ut::Vector<2>(1, 1);
	v[23].position = ut::Vector<3>(-x,  y, z); v[23].normal = ut::Vector<3>(0, 0, 1); v[23].texcoord = ut::Vector<2>(0, 1);

	for (ut::uint32 i = 0; i < vertex_count; i++)
	{
		v[i].position += position;
	}

	ut::Result<Buffer, ut::Error> vertex_buffer = device.CreateBuffer(ut::Move(vertex_buffer_info));
	if (!vertex_buffer)
	{
		return ut::MakeError(vertex_buffer.MoveAlt());
	}

	Buffer::Info index_buffer_info;
	index_buffer_info.type = Buffer::index;
	index_buffer_info.usage = render::memory::gpu_immutable;
	index_buffer_info.size = sizeof(ut::uint32) * face_count * 3;
	index_buffer_info.stride = sizeof(ut::uint32);
	index_buffer_info.data.Resize(index_buffer_info.size);
	ut::uint32* indices = reinterpret_cast<ut::uint32*>(index_buffer_info.data.GetAddress());

	indices[0] = 3;   indices[1] = 1;   indices[2] = 0;
	indices[3] = 2;   indices[4] = 1;   indices[5] = 3;

	indices[6] = 6;   indices[7]  = 4;  indices[8]  = 5;
	indices[9] = 7;   indices[10] = 4;  indices[11] = 6;

	indices[12] = 11; indices[13] = 9;  indices[14] = 8;
	indices[15] = 10; indices[16] = 9;  indices[17] = 11;

	indices[18] = 14; indices[19] = 12; indices[20] = 13;
	indices[21] = 15; indices[22] = 12; indices[23] = 14;

	indices[24] = 19; indices[25] = 17; indices[26] = 16;
	indices[27] = 18; indices[28] = 17; indices[29] = 19;

	indices[30] = 22; indices[31] = 20; indices[32] = 21;
	indices[33] = 23; indices[34] = 20; indices[35] = 22;

	ut::Result<Buffer, ut::Error> index_buffer = device.CreateBuffer(ut::Move(index_buffer_info));
	if (!index_buffer)
	{
		return ut::MakeError(index_buffer.MoveAlt());
	}

	InputAssemblyState primitive_info;
	primitive_info.topology = primitive::triangle_list;
	primitive_info.stride = CubeVertex::size;
	primitive_info.elements = CubeVertex::CreateLayout();

	return AddResource<Mesh>(Mesh(face_count, vertex_count,
	                              vertex_buffer.Move(),
	                              index_buffer.Move(),
	                              index_type_uint32,
	                              ut::Move(primitive_info)));
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