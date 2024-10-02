//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_rc_mgr.h"
#include "systems/render/engine/ve_render_frame.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Helper function to compute tangents.
//    @param vertices - a pointer to the first vertice, all vertices must have
//                      initialized position and texture coordinats.
//    @param indices - a pointer to the first index.
//    @param vertex_count - the number of vertices to initialize.
//    @param index_count - the number of indices.
template<typename VertexType, typename IndexType = ut::uint32>
void ComputeTangents(VertexType* vertices,
                     IndexType* indices,
                     size_t vertex_count,
                     size_t index_count)
{
	for (size_t i = 0; i < index_count; i = i + 3)
	{
		VertexType& v0 = vertices[indices[i]];
		VertexType& v1 = vertices[indices[i + 1]];
		VertexType& v2 = vertices[indices[i + 2]];
		
		const ut::Vector<3> positions[3] = { v0.position, v1.position, v2.position };
		const ut::Vector<2> texcoords[3] = { v0.texcoord, v1.texcoord, v2.texcoord };
		const ut::Vector<3> d1 = positions[2] - positions[0];
		const ut::Vector<3> d2 = positions[1] - positions[0];
		const ut::Vector<2> lt1(texcoords[1].X() - texcoords[0].X(), texcoords[1].Y() - texcoords[0].Y());
		const ut::Vector<2> lt2(texcoords[2].X() - texcoords[0].X(), texcoords[2].Y() - texcoords[0].Y());
		const float r = lt1.X() * lt2.Y() - lt2.X() * lt1.Y();
		const float tmp = ut::Abs(r) <= 0.0001f ? 1.0f : (1.0f / r);

		ut::Vector<3> tangent;
		tangent.X() = lt2.Y() * d2.X() - lt1.Y() * d1.X();
		tangent.Y() = lt2.Y() * d2.Y() - lt1.Y() * d1.Y();
		tangent.Z() = lt2.Y() * d2.Z() - lt1.Y() * d1.Z();
		tangent = tangent.ElementWise() * tmp;
		if (!ut::Equal(tangent.Length(), 0.0f))
		{
			tangent = tangent.Normalize();
		}
		
		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
	}

	// normalize tangents
	for (size_t i = 0; i < vertex_count; i++)
	{
		vertices[i].tangent = vertices[i].tangent.Normalize();
	}
}

//----------------------------------------------------------------------------//
// Constructor.
ResourceManager::ResourceManager(Device& device_ref,
                                 const Config<Settings>& cfg) : device(device_ref)
                                                              , frame_counter(0)
                                                              , frames_in_flight(cfg.frames_in_flight)
                                                              , garbage(cfg.frames_in_flight)
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
// Creates vertex buffer representing a fullscreen quad.
ut::Result<RcRef<Mesh>, ut::Error> ResourceManager::CreateRect(const ut::Vector<2>& position,
                                                               const ut::Vector<2>& extent)
{
	constexpr Mesh::VertexFormat vertex_format = Mesh::vertex_pos2_texcoord2_float;
	typedef MeshVertex<vertex_format>::Type QuadVertex;

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

	Mesh mesh(2, 6,
	          vertex_buffer.Move(),
	          ut::Optional<Buffer>(),
	          index_type_uint32,
	          vertex_format,
	          ut::Array<Mesh::Subset>());

	return AddResource<Mesh>(ut::Move(mesh));
}

//----------------------------------------------------------------------------->
// Creates a mesh representing a box.
ut::Result<RcRef<Mesh>, ut::Error> ResourceManager::CreateBox(const ut::Vector<3>& position,
                                                              const ut::Vector<3>& extent,
                                                              ut::Optional<ut::String> name)
{
	constexpr Mesh::VertexFormat vertex_format = Mesh::vertex_pos3_texcoord2_normal3_tangent3_float;
	typedef MeshVertex<vertex_format>::Type CubeVertex;

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

	Buffer::Info index_buffer_info;
	index_buffer_info.type = Buffer::index;
	index_buffer_info.usage = render::memory::gpu_immutable;
	index_buffer_info.size = sizeof(ut::uint32) * face_count * 3;
	index_buffer_info.stride = sizeof(ut::uint32);
	index_buffer_info.data.Resize(index_buffer_info.size);
	ut::uint32* indices = reinterpret_cast<ut::uint32*>(index_buffer_info.data.GetAddress());

	// indices
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

	// tangents
	ComputeTangents<CubeVertex::Type>(v, indices, vertex_count, face_count * 3);

	// create vertex buffer
	ut::Result<Buffer, ut::Error> vertex_buffer = device.CreateBuffer(ut::Move(vertex_buffer_info));
	if (!vertex_buffer)
	{
		return ut::MakeError(vertex_buffer.MoveAlt());
	}

	// create index buffer
	ut::Result<Buffer, ut::Error> index_buffer = device.CreateBuffer(ut::Move(index_buffer_info));
	if (!index_buffer)
	{
		return ut::MakeError(index_buffer.MoveAlt());
	}

	// create material
	ut::Result<Material, ut::Error> default_material = CreateDefaultMaterial();
	if (!default_material)
	{
		return ut::MakeError(default_material.MoveAlt());
	}

	// initialize subsets
	ut::Array<Mesh::Subset> subsets;
	Mesh::Subset subset;
	subset.index_offset = 0;
	subset.index_count = face_count * Mesh::skPolygonVertices;
	subset.material = default_material.Move();
	subsets.Add(ut::Move(subset));

	// create mesh
	Mesh mesh(face_count, vertex_count,
	          vertex_buffer.Move(),
	          index_buffer.Move(),
	          index_type_uint32,
	          vertex_format,
	          ut::Move(subsets));

	return AddResource<Mesh>(ut::Move(mesh), ut::Move(name));
}

// Creates a mesh representing a sphere.
ut::Result<RcRef<Mesh>, ut::Error> ResourceManager::CreateSphere(const ut::Vector<3>& position,
                                                                 float radius,
                                                                 ut::uint32 segment_count,
                                                                 ut::Optional<ut::String> name)
{
	constexpr Mesh::VertexFormat vertex_format = Mesh::vertex_pos3_texcoord2_normal3_tangent3_float;
	typedef MeshVertex<vertex_format>::Type SphereVertex;

	const ut::uint32 nv = segment_count;
	const ut::uint32 nh = segment_count;
	const ut::uint32 vertex_count = (nv + 1) * (nh + 1);
	const ut::uint32 face_count = nv*nh * 2;

	// vertex buffer info
	Buffer::Info vertex_buffer_info;
	vertex_buffer_info.type = Buffer::vertex;
	vertex_buffer_info.usage = render::memory::gpu_immutable;
	vertex_buffer_info.size = SphereVertex::size * vertex_count;
	vertex_buffer_info.stride = SphereVertex::size;
	vertex_buffer_info.data.Resize(vertex_buffer_info.size);
	SphereVertex::Type* v = reinterpret_cast<SphereVertex::Type*>(vertex_buffer_info.data.GetAddress());

	// initialize vertices
	for (ut::uint32 i = 0; i < nv + 1; i++)
	{
		const float i_nv = static_cast<float>(i) / static_cast<float>(nv);
		const float AngleXZ = 2.0f * ut::Precision<float>::pi * i_nv;
		for (ut::uint32 j = 0; j < nh + 1; j++)
		{
			const float j_nh = static_cast<float>(j) / static_cast<float>(nh);
			const float AngleY = ut::Precision<float>::pi * j_nh;

			SphereVertex::Type& vertex = v[i*(nh + 1) + j];

			vertex.position.X() = ut::Cos(AngleXZ) * ut::Sin(AngleY) * radius;
			vertex.position.Y() = ut::Cos(AngleY) * radius;
			vertex.position.Z() = ut::Sin(AngleXZ) * ut::Sin(AngleY) * radius;

			vertex.normal = vertex.position.Normalize();

			vertex.texcoord.X() = i_nv;
			vertex.texcoord.Y() = 1.0f - j_nh;
		}
	}

	// apply vertex offset
	for (ut::uint32 i = 0; i < vertex_count; i++)
	{
		v[i].position += position;
	}

	// index buffer info
	Buffer::Info index_buffer_info;
	index_buffer_info.type = Buffer::index;
	index_buffer_info.usage = render::memory::gpu_immutable;
	index_buffer_info.size = sizeof(ut::uint32) * face_count * 3;
	index_buffer_info.stride = sizeof(ut::uint32);
	index_buffer_info.data.Resize(index_buffer_info.size);
	ut::uint32* indices = reinterpret_cast<ut::uint32*>(index_buffer_info.data.GetAddress());

	// initialize indices
	ut::uint32 ind = 0;
	for (ut::uint32 i = 0; i < nv; i++)
	{
		for (ut::uint32 j = 0; j < nh; j++)
		{
			const ut::uint32 vi = (nh + 1)*i + j;
			const ut::uint32 di = (nh + 1);

			indices[3 * ind + 1] = vi;
			indices[3 * ind + 2] = vi + di;
			indices[3 * ind + 0] = vi + di + 1;
			indices[3 * (ind + 1) + 2] = vi;
			indices[3 * (ind + 1) + 1] = vi + 1;
			indices[3 * (ind + 1) + 0] = vi + di + 1;

			ind += 2;
		}
	}

	// tangents
	ComputeTangents<SphereVertex::Type>(v, indices, vertex_count, face_count * 3);

	// create vertex buffer
	ut::Result<Buffer, ut::Error> vertex_buffer = device.CreateBuffer(ut::Move(vertex_buffer_info));
	if (!vertex_buffer)
	{
		return ut::MakeError(vertex_buffer.MoveAlt());
	}

	// create index buffer
	ut::Result<Buffer, ut::Error> index_buffer = device.CreateBuffer(ut::Move(index_buffer_info));
	if (!index_buffer)
	{
		return ut::MakeError(index_buffer.MoveAlt());
	}

	// create material
	ut::Result<Material, ut::Error> default_material = CreateDefaultMaterial();
	if (!default_material)
	{
		return ut::MakeError(default_material.MoveAlt());
	}

	// initialize subsets
	ut::Array<Mesh::Subset> subsets;
	Mesh::Subset subset;
	subset.index_offset = 0;
	subset.index_count = face_count * Mesh::skPolygonVertices;
	subset.material = default_material.Move();
	subsets.Add(ut::Move(subset));

	// create mesh
	Mesh mesh(face_count, vertex_count,
	          vertex_buffer.Move(),
	          index_buffer.Move(),
	          index_type_uint32,
	          vertex_format,
	          ut::Move(subsets));

	return AddResource<Mesh>(ut::Move(mesh), ut::Move(name));
}

// Creates a mesh representing a torus.
ut::Result<RcRef<Mesh>, ut::Error> ResourceManager::CreateTorus(const ut::Vector<3>& position,
                                                                float radius,
                                                                float tube_radius,
                                                                ut::uint32 radial_segment_count,
                                                                ut::uint32 tubular_segment_count,
                                                                ut::Optional<ut::String> name)
{
	constexpr Mesh::VertexFormat vertex_format = Mesh::vertex_pos3_texcoord2_normal3_tangent3_float;
	typedef MeshVertex<vertex_format>::Type TorusVertex;

	const ut::uint32 vertex_count = (radial_segment_count + 1) * (tubular_segment_count + 1);
	const ut::uint32 face_count = radial_segment_count * tubular_segment_count * 2;

	// vertex buffer info
	Buffer::Info vertex_buffer_info;
	vertex_buffer_info.type = Buffer::vertex;
	vertex_buffer_info.usage = render::memory::gpu_immutable;
	vertex_buffer_info.size = TorusVertex::size * vertex_count;
	vertex_buffer_info.stride = TorusVertex::size;
	vertex_buffer_info.data.Resize(vertex_buffer_info.size);
	TorusVertex::Type* v = reinterpret_cast<TorusVertex::Type*>(vertex_buffer_info.data.GetAddress());

	// index buffer info
	Buffer::Info index_buffer_info;
	index_buffer_info.type = Buffer::index;
	index_buffer_info.usage = render::memory::gpu_immutable;
	index_buffer_info.size = sizeof(ut::uint32) * face_count * Mesh::skPolygonVertices;
	index_buffer_info.stride = sizeof(ut::uint32);
	index_buffer_info.data.Resize(index_buffer_info.size);
	ut::uint32* indices = reinterpret_cast<ut::uint32*>(index_buffer_info.data.GetAddress());

	// initialize vertices
	const float radialStep = 2.0f * ut::Precision<float>::pi / static_cast<float>(radial_segment_count);
	const float tubularStep = 2.0f * ut::Precision<float>::pi / static_cast<float>(tubular_segment_count);
	for (ut::uint32 i = 0; i <= radial_segment_count; ++i)
	{
		const float u = static_cast<float>(i) * radialStep;
		const float cosU = ut::Cos(u);
		const float sinU = ut::Sin(u);

		for (ut::uint32 j = 0; j <= tubular_segment_count; ++j)
		{
			const float vrad = static_cast<float>(j) * tubularStep;
			const float cosV = ut::Cos(vrad);
			const float sinV = ut::Sin(vrad);

			TorusVertex::Type& vertex = v[i * (tubular_segment_count + 1) + j];

			vertex.position.X() = (radius + tube_radius * cosV) * cosU;
			vertex.position.Y() = (radius + tube_radius * cosV) * sinU;
			vertex.position.Z() = tube_radius * sinV;

			vertex.normal.X() = cosV * cosU;
			vertex.normal.Y() = cosV * sinU;
			vertex.normal.Z() = sinV;

			vertex.texcoord.X() = static_cast<float>(i) / static_cast<float>(radial_segment_count);
			vertex.texcoord.Y() = static_cast<float>(j) / static_cast<float>(tubular_segment_count);

			// Add indices for a quad (two triangles)
			if (i < radial_segment_count && j < tubular_segment_count)
			{
				ut::uint32 a = i * (tubular_segment_count + 1) + j;
				ut::uint32 b = (i + 1) * (tubular_segment_count + 1) + j;
				ut::uint32 c = (i + 1) * (tubular_segment_count + 1) + (j + 1);
				ut::uint32 d = i * (tubular_segment_count + 1) + (j + 1);

				indices[0] = a;
				indices[1] = b;
				indices[2] = c;

				indices[3] = a;
				indices[4] = c;
				indices[5] = d;

				indices += 6;
			}
		}
	}

	// get back initial index address
	indices = reinterpret_cast<ut::uint32*>(index_buffer_info.data.GetAddress());

	// apply vertex offset
	for (ut::uint32 i = 0; i < vertex_count; i++)
	{
		v[i].position += position;
	}

	// tangents
	ComputeTangents<TorusVertex::Type>(v, indices, vertex_count, face_count * Mesh::skPolygonVertices);

	// create vertex buffer
	ut::Result<Buffer, ut::Error> vertex_buffer = device.CreateBuffer(ut::Move(vertex_buffer_info));
	if (!vertex_buffer)
	{
		return ut::MakeError(vertex_buffer.MoveAlt());
	}

	// create index buffer
	ut::Result<Buffer, ut::Error> index_buffer = device.CreateBuffer(ut::Move(index_buffer_info));
	if (!index_buffer)
	{
		return ut::MakeError(index_buffer.MoveAlt());
	}

	// create material
	ut::Result<Material, ut::Error> default_material = CreateDefaultMaterial();
	if (!default_material)
	{
		return ut::MakeError(default_material.MoveAlt());
	}

	// initialize subsets
	ut::Array<Mesh::Subset> subsets;
	Mesh::Subset subset;
	subset.index_offset = 0;
	subset.index_count = face_count * Mesh::skPolygonVertices;
	subset.material = default_material.Move();
	subsets.Add(ut::Move(subset));

	// create mesh
	Mesh mesh(face_count, vertex_count,
	          vertex_buffer.Move(),
	          index_buffer.Move(),
	          index_type_uint32,
	          vertex_format,
	          ut::Move(subsets));

	return AddResource<Mesh>(ut::Move(mesh), ut::Move(name));
}

//----------------------------------------------------------------------------->
// Creates an image filled with solid color.
ut::Result<RcRef<Map>, ut::Error> ResourceManager::CreateImage(ut::uint32 width,
                                                               ut::uint32 height,
                                                               const ut::Color<4, ut::byte>& color,
                                                               ut::Optional<ut::String> name,
                                                               bool is_cubemap)
{
	Image::Info img_info;
	img_info.type = is_cubemap ? Image::type_cube : Image::type_2D;
	img_info.format = pixel::r8g8b8a8_unorm;
	img_info.usage = render::memory::gpu_immutable;
	img_info.width = width;
	img_info.height = height;
	img_info.depth = 1;
	img_info.mip_count = 1;

	const ut::uint32 face_count = is_cubemap ? 6 : 1;
	const size_t face_size = img_info.width * img_info.height;
	img_info.data.Resize(face_count * face_size * pixel::GetSize(img_info.format));
	ut::Color<4, ut::byte>* pixels = reinterpret_cast<ut::Color<4, ut::byte>*>(img_info.data.GetAddress());
	for (ut::uint32 face = 0; face < face_count; face++)
	{
		for (size_t i = 0; i < img_info.height; i++)
		{
			for (size_t j = 0; j < img_info.width; j++)
			{
				pixels[face * face_size + i * img_info.width + j] = color;
			}
		}
	}

	ut::Result<Image, ut::Error> image = device.CreateImage(ut::Move(img_info));
	if (!image)
	{
		return ut::MakeError(image.MoveAlt());
	}

	return AddResource<Map>(Map(image.Move()), name);
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
	// lock resources
	ut::ScopeRWLock scope_lock(lock, ut::access_write);

	// increment frame counter
	if (++frame_counter >= frames_in_flight)
	{
		frame_counter = 0;
	}

	// delete unused resources
	garbage[frame_counter].Reset();
}

//----------------------------------------------------------------------------->
// Creates a default material (white, roughness is 1, albedo is 0)
ut::Result<Material, ut::Error> ResourceManager::CreateDefaultMaterial()
{
	const ut::String engine_rc_dir(engine_rc::skDir);
	ut::Result<RcRef<Map>, ut::Error> diffuse_map = Find<Map>(engine_rc_dir + engine_rc::skWhite);
	if (!diffuse_map)
	{
		return ut::MakeError(diffuse_map.MoveAlt());
	}

	ut::Result<RcRef<Map>, ut::Error> normal_map = Find<Map>(engine_rc_dir + engine_rc::skNormal);
	if (!normal_map)
	{
		return ut::MakeError(normal_map.MoveAlt());
	}

	ut::Result<RcRef<Map>, ut::Error> material_map = Find<Map>(engine_rc_dir + engine_rc::skRed);
	if (!material_map)
	{
		return ut::MakeError(material_map.MoveAlt());
	}

	Material material;
	material.diffuse = diffuse_map.Move();
	material.normal = normal_map.Move();
	material.material = material_map.Move();
	material.alpha = Material::alpha_opaque;
	material.double_sided = false;

	return material;
}

//----------------------------------------------------------------------------->
// Creates internal engine resources (primitives, common 1x1 textures, etc.)
ut::Optional<ut::Error> ResourceManager::CreateEngineResources()
{
	const ut::String engine_rc_dir(engine_rc::skDir);

	// fullscreen rect
	ut::Result<RcRef<Mesh>, ut::Error> mesh = CreateRect(ut::Vector<2>(0), ut::Vector<2>(1));
	if (!mesh)
	{
		return mesh.MoveAlt();
	}
	fullscreen_quad = mesh.Move();

	// 1x1 black image
	ut::Result<RcRef<Map>, ut::Error> map = CreateImage(1, 1, ut::Color<4, ut::byte>(0, 0, 0, 255),
	                                                    engine_rc_dir + engine_rc::skBlack);
	if (!map)
	{
		return map.MoveAlt();
	}
	img_black = map.Move();

	// 1x1 white image
	map = CreateImage(1, 1, ut::Color<4, ut::byte>(255, 255, 255, 255),
	                  engine_rc_dir + engine_rc::skWhite);
	if (!map)
	{
		return map.MoveAlt();
	}
	img_white = map.Move();

	// 1x1 red image
	map = CreateImage(1, 1, ut::Color<4, ut::byte>(255, 0, 0, 255),
	                  engine_rc_dir + engine_rc::skRed);
	if (!map)
	{
		return map.MoveAlt();
	}
	img_red = map.Move();

	// 1x1 green image
	map = CreateImage(1, 1, ut::Color<4, ut::byte>(0, 255, 0, 255),
	                  engine_rc_dir + engine_rc::skGreen);
	if (!map)
	{
		return map.MoveAlt();
	}
	img_green = map.Move();

	// 1x1 blue image
	map = CreateImage(1, 1, ut::Color<4, ut::byte>(0, 0, 255, 255),
	                  engine_rc_dir + engine_rc::skBlue);
	if (!map)
	{
		return map.MoveAlt();
	}
	img_blue = map.Move();

	// 1x1 normal map
	map = CreateImage(1, 1, ut::Color<4, ut::byte>(127, 127, 255, 255),
	                  engine_rc_dir + engine_rc::skNormal);
	if (!map)
	{
		return map.MoveAlt();
	}
	img_normal = map.Move();

	// cube
	mesh = CreateBox(ut::Vector<3>(0), ut::Vector<3>(1), engine_rc_dir + engine_rc::skBox);
	if (!mesh)
	{
		return mesh.MoveAlt();
	}
	cube = mesh.Move();

	// sphere
	mesh = CreateSphere(ut::Vector<3>(0), 1.0f, 16, engine_rc_dir + engine_rc::skSphere);
	if (!mesh)
	{
		return mesh.MoveAlt();
	}
	sphere = mesh.Move();

	// torus
	mesh = CreateTorus(ut::Vector<3>(0), 1.0f, 0.5f, 32, 32, engine_rc_dir + engine_rc::skTorus);
	if (!mesh)
	{
		return mesh.MoveAlt();
	}
	tor = mesh.Move();

	// success
	return ut::Optional<ut::Error>();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//