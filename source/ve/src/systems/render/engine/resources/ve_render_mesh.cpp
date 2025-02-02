//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/resources/ve_render_mesh.h"
#include "systems/render/engine/ve_render_rc_mgr.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Resource, ve::render::Mesh, "mesh")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Instance id is a per-instance vertex element to
// identify current instance in shader.
const char* Mesh::skInstanceIdSemantic = "INSTANCE_ID";

// The list of names acceptable for generating a mesh from a generator prompt.
const char* ResourceCreator<Mesh>::skTypeBox = "box";
const char* ResourceCreator<Mesh>::skTypeSphere = "sphere";
const char* ResourceCreator<Mesh>::skTypeTorus = "torus";

//----------------------------------------------------------------------------//
// The MeshVertexHelper is a helper struct that provides recursive functionality
// for iterating vertex formats.
template<size_t id>
struct MeshVertexHelper
{
	typedef typename MeshVertex<static_cast<Mesh::VertexFormat>(id)>::Type VertexType;

	static ut::Optional<ut::Error> InitializeIaState(Mesh::VertexFormat format, InputAssemblyState& input_assembly)
	{
		if (format == static_cast<Mesh::VertexFormat>(id))
		{
			input_assembly.vertex_stride = VertexType::size;
			input_assembly.elements = VertexType::CreateLayout();
			return ut::Optional<ut::Error>();
		}

		return MeshVertexHelper<id - 1>::InitializeIaState(format, input_assembly);
	}
};

// Specializtion for the last element.
template<> struct MeshVertexHelper<0>
{
	typedef typename MeshVertex<static_cast<Mesh::VertexFormat>(0)>::Type VertexType;

	static ut::Optional<ut::Error> InitializeIaState(Mesh::VertexFormat format, InputAssemblyState& input_assembly)
	{
		if (format == static_cast<Mesh::VertexFormat>(0))
		{
			input_assembly.vertex_stride = VertexType::size;
			input_assembly.elements = VertexType::CreateLayout();
			return ut::Optional<ut::Error>();
		}

		return ut::Error(ut::error::not_supported, "Unsupported mesh vertex format.");
	}
};

//----------------------------------------------------------------------------//
// Constructor.
Mesh::Mesh(ut::uint32 in_polygon_count,
           ut::uint32 in_vertex_count,
           Buffer in_vertex_buffer,
           ut::Optional<Buffer> in_index_buffer,
           IndexType in_index_type,
           VertexFormat in_vertex_format,
           PolygonMode in_polygon_mode,
           ut::Array<Subset> in_subsets) : polygon_count(in_polygon_count)
                                         , vertex_count(in_vertex_count)
                                         , vertex_buffer(ut::Move(in_vertex_buffer))
                                         , index_buffer(ut::Move(in_index_buffer))
                                         , index_type(in_index_type)
                                         , vertex_format(in_vertex_format)
                                         , polygon_mode(in_polygon_mode)
                                         , subsets(ut::Move(in_subsets))
{}

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& Mesh::Identify() const
{
	return ut::Identify(this);
}

// Creates the input assembly state from the provided vertex format.
InputAssemblyState Mesh::CreateIaState(VertexFormat vertex_format,
                                       PolygonMode polygon_mode,
                                       Instancing instancing)
{
	InputAssemblyState ia_state;
	switch (polygon_mode)
	{
		case Mesh::PolygonMode::line: ia_state.topology = primitive::line_list; break;
		case Mesh::PolygonMode::triangle: ia_state.topology = primitive::triangle_list; break;
		case Mesh::PolygonMode::triangle_wireframe: ia_state.topology = primitive::triangle_list; break;
		default: ia_state.topology = primitive::triangle_list;
	}

	ut::Optional<ut::Error> init_error = MeshVertexHelper<static_cast<size_t>(VertexFormat::count) - 1>::
		InitializeIaState(vertex_format, ia_state);
	UT_ASSERT(!init_error);

	if (instancing == Instancing::on)
	{
		VertexElement instance_id(skInstanceIdSemantic, skInstanceIdFormat, 0);
		ia_state.instance_stride = pixel::GetSize(skInstanceIdFormat);
		ia_state.instance_elements.Add(ut::Move(instance_id));
	}

	return ia_state;
}

// Creates the input assembly state for the desired subset of this mesh.
InputAssemblyState Mesh::CreateIaState(Instancing instancing) const
{
	return CreateIaState(vertex_format, polygon_mode, instancing);
}

// Converts provided Mesh::PolygonMode value to corresponding
// RasterizationState::PolygonMode value.
RasterizationState::PolygonMode Mesh::GetRasterizerPolygonMode(PolygonMode polygon_mode)
{
	switch (polygon_mode)
	{
		case Mesh::PolygonMode::line: return RasterizationState::line;
		case Mesh::PolygonMode::triangle: return RasterizationState::fill;
		case Mesh::PolygonMode::triangle_wireframe: return RasterizationState::line;
		default: return RasterizationState::fill;
	}
}

// Returns an array of shader macros for the specified vertex format.
ut::Array<Shader::MacroDefinition> Mesh::GenerateVertexMacros(VertexFormat vertex_format,
                                                              Instancing instancing)
{
	// detect what components (position, texcoord, etc.) are available for
	// the provided vertex format
	ut::Optional<const VertexElement&> component_map[vertex_traits::component_count];
	InputAssemblyState ia = CreateIaState(vertex_format, PolygonMode::triangle, instancing);
	vertex_traits::ComponentIterator<vertex_traits::component_count - 1>::Map(component_map, ia);
	
	const ut::String availability_prefix = "VERTEX_HAS_";

	ut::Array<Shader::MacroDefinition> macros;
	const size_t element_count = ia.elements.Count();
	for (size_t element_id = 0; element_id < element_count; element_id++)
	{
		const VertexElement& element = ia.elements[element_id];
		for (ut::uint32 component_id = 0; component_id < vertex_traits::component_count; component_id++)
		{
			const ut::Optional<const VertexElement&>& component = component_map[component_id];
			if (!component)
			{
				continue;
			}
			
			if (element.semantic_name == component->semantic_name)
			{
				// component availability (VERTEX_HAS_POSITION, VERTEX_HAS_TEXCOORD, etc.)
				Shader::MacroDefinition macro;
				macro.name = availability_prefix + element.semantic_name;
				macro.value = "1";
				macros.Add(macro);

				// position dimension
				if (component_id == vertex_traits::position)
				{
					const bool is_2d = pixel::GetSize(element.format) == sizeof(ut::Vector<2, float>);
					macro.name = ut::String("VERTEX_") + (is_2d ? "2" : "3") + "D_POSITION";
					macro.value = "1";
					macros.Add(macro);
				}

				break;
			}
		}
	}

	// instancing
	if (instancing == Instancing::on)
	{
		Shader::MacroDefinition macro;
		macro.name = availability_prefix + skInstanceIdSemantic;
		macro.value = "1";
		macros.Add(macro);
	}

	return macros;
}

// Returns a number of vertices to render one polygon using the provided mode.
ut::uint32 Mesh::GetPolygonVertexCount(PolygonMode mode)
{
	switch (mode)
	{
		case PolygonMode::line: return skLineVertices;
		case PolygonMode::triangle: return skTriangleVertices;
		case PolygonMode::triangle_wireframe: return skTriangleVertices;
		default: return skTriangleVertices;
	}
}

//----------------------------------------------------------------------------//
// Constructor initializes a hashmap of generation functions in order to be
// able quickly pick the correct one (instead of comparing all mesh names in
// brute-force manner) when user calls Create() method.
ResourceCreator<Mesh>::ResourceCreator(Device& in_device,
                                       ResourceManager& in_rc_mgr) : device(in_device)
                                                                   , rc_mgr(in_rc_mgr)
{
	generators.Insert(skTypeBox, CreateBox);
	generators.Insert(skTypeSphere, CreateSphere);
	generators.Insert(skTypeTorus, CreateTorus);
}

// Creates a mesh according to the provided name, path or generator prompt.
ut::Result<RcRef<Mesh>, ut::Error> ResourceCreator<Mesh>::Create(const ut::String& name)
{
	const bool is_generator_prompt = Resource::GeneratorPrompt::Check(name);
	if (!is_generator_prompt)
	{
		return ut::MakeError(ut::error::not_implemented);
	}

	Resource::GeneratorPrompt::Attributes generator_attributes;
	ut::Result<ut::String, ut::Error> prompt_parse_result = Resource::GeneratorPrompt::Parse(name,
	                                                                                         generator_attributes);
	if (!prompt_parse_result)
	{
		return ut::MakeError(prompt_parse_result.MoveAlt());
	}

	ut::Optional<ut::Function<Generator>&> find_result = generators.Find(prompt_parse_result.Get());
	if (!find_result)
	{
		return ut::MakeError(ut::error::not_implemented);
	}

	const ut::Function<Generator>& generator = find_result.Get();
	return generator(name, generator_attributes, device, rc_mgr);
}

// Creates a 2D rectangle with vertex format
// ve::render::Mesh::vertex_pos2_texcoord2_float.
ut::Result<RcRef<Mesh>, ut::Error> ResourceCreator<Mesh>::CreateEyeSpaceRect(const ut::Vector<2>& position,
                                                                             const ut::Vector<2>& extent)
{
	constexpr Mesh::VertexFormat vertex_format = Mesh::VertexFormat::pos2_texcoord2_float;
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
	          Mesh::PolygonMode::triangle,
	          ut::Array<Mesh::Subset>());

	return rc_mgr.AddResource<Mesh>(ut::Move(mesh));
}

// Generates vertices for the box mesh.
// 	  @param vertex_format - desired vertex format of the final mesh.
//    @param polygon_mode - desired polygon mode of the final mesh.
// 	  @param offset - offset applied to all vertices.
//    @param extent - extent of the box.
//    @return - ve::render::ResourceCreator<Mesh>::Geometry data object.
ResourceCreator<Mesh>::GeometryData ResourceCreator<Mesh>::GenBoxVertices(Mesh::VertexFormat vertex_format,
                                                                          Mesh::PolygonMode polygon_mode,
                                                                          const ut::Vector<3>& offset,
                                                                          const ut::Vector<3>& extent)
{
	ResourceCreator<Mesh>::GeometryData geometry;
	const InputAssemblyState input_assembly = Mesh::CreateIaState(vertex_format, Mesh::PolygonMode::triangle);

	// set vertices
	geometry.vertex_count = 24;
	geometry.vertex_buffer.Resize(input_assembly.vertex_stride * geometry.vertex_count);
	VertexReflector vertices(input_assembly, geometry.vertex_buffer.GetAddress());

	vertices.Get<vertex_traits::position>(0).Write(ut::Vector<3>(-extent.X(), extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(0).Write(ut::Vector<3>(0, 1, 0));
	vertices.Get<vertex_traits::texcoord>(0).Write(ut::Vector<2>(0, 0));

	vertices.Get<vertex_traits::position>(1).Write(ut::Vector<3>(extent.X(), extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(1).Write(ut::Vector<3>(0, 1, 0));
	vertices.Get<vertex_traits::texcoord>(1).Write(ut::Vector<2>(1, 0));

	vertices.Get<vertex_traits::position>(2).Write(ut::Vector<3>(extent.X(), extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(2).Write(ut::Vector<3>(0, 1, 0));
	vertices.Get<vertex_traits::texcoord>(2).Write(ut::Vector<2>(1, 1));

	vertices.Get<vertex_traits::position>(3).Write(ut::Vector<3>(-extent.X(), extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(3).Write(ut::Vector<3>(0, 1, 0));
	vertices.Get<vertex_traits::texcoord>(3).Write(ut::Vector<2>(0, 1));

	vertices.Get<vertex_traits::position>(4).Write(ut::Vector<3>(-extent.X(), -extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(4).Write(ut::Vector<3>(0, -1, 0));
	vertices.Get<vertex_traits::texcoord>(4).Write(ut::Vector<2>(0, 0));

	vertices.Get<vertex_traits::position>(5).Write(ut::Vector<3>(extent.X(), -extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(5).Write(ut::Vector<3>(0, -1, 0));
	vertices.Get<vertex_traits::texcoord>(5).Write(ut::Vector<2>(1, 0));

	vertices.Get<vertex_traits::position>(6).Write(ut::Vector<3>(extent.X(), -extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(6).Write(ut::Vector<3>(0, -1, 0));
	vertices.Get<vertex_traits::texcoord>(6).Write(ut::Vector<2>(1, 1));

	vertices.Get<vertex_traits::position>(7).Write(ut::Vector<3>(-extent.X(), -extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(7).Write(ut::Vector<3>(0, -1, 0));
	vertices.Get<vertex_traits::texcoord>(7).Write(ut::Vector<2>(0, 1));

	vertices.Get<vertex_traits::position>(8).Write(ut::Vector<3>(-extent.X(), -extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(8).Write(ut::Vector<3>(-1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(8).Write(ut::Vector<2>(0, 1));

	vertices.Get<vertex_traits::position>(9).Write(ut::Vector<3>(-extent.X(), -extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(9).Write(ut::Vector<3>(-1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(9).Write(ut::Vector<2>(1, 1));

	vertices.Get<vertex_traits::position>(10).Write(ut::Vector<3>(-extent.X(), extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(10).Write(ut::Vector<3>(-1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(10).Write(ut::Vector<2>(1, 0));

	vertices.Get<vertex_traits::position>(11).Write(ut::Vector<3>(-extent.X(), extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(11).Write(ut::Vector<3>(-1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(11).Write(ut::Vector<2>(0, 0));

	vertices.Get<vertex_traits::position>(12).Write(ut::Vector<3>(extent.X(), -extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(12).Write(ut::Vector<3>(1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(12).Write(ut::Vector<2>(1, 1));

	vertices.Get<vertex_traits::position>(13).Write(ut::Vector<3>(extent.X(), -extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(13).Write(ut::Vector<3>(1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(13).Write(ut::Vector<2>(0, 1));

	vertices.Get<vertex_traits::position>(14).Write(ut::Vector<3>(extent.X(), extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(14).Write(ut::Vector<3>(1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(14).Write(ut::Vector<2>(0, 0));

	vertices.Get<vertex_traits::position>(15).Write(ut::Vector<3>(extent.X(), extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(15).Write(ut::Vector<3>(1, 0, 0));
	vertices.Get<vertex_traits::texcoord>(15).Write(ut::Vector<2>(1, 0));

	vertices.Get<vertex_traits::position>(16).Write(ut::Vector<3>(-extent.X(), -extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(16).Write(ut::Vector<3>(0, 0, -1));
	vertices.Get<vertex_traits::texcoord>(16).Write(ut::Vector<2>(0, 1));

	vertices.Get<vertex_traits::position>(17).Write(ut::Vector<3>(extent.X(), -extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(17).Write(ut::Vector<3>(0, 0, -1));
	vertices.Get<vertex_traits::texcoord>(17).Write(ut::Vector<2>(1, 1));

	vertices.Get<vertex_traits::position>(18).Write(ut::Vector<3>(extent.X(), extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(18).Write(ut::Vector<3>(0, 0, -1));
	vertices.Get<vertex_traits::texcoord>(18).Write(ut::Vector<2>(1, 0));

	vertices.Get<vertex_traits::position>(19).Write(ut::Vector<3>(-extent.X(), extent.Y(), -extent.Z()));
	vertices.Get<vertex_traits::normal>(19).Write(ut::Vector<3>(0, 0, -1));
	vertices.Get<vertex_traits::texcoord>(19).Write(ut::Vector<2>(0, 0));

	vertices.Get<vertex_traits::position>(20).Write(ut::Vector<3>(-extent.X(), -extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(20).Write(ut::Vector<3>(0, 0, 1));
	vertices.Get<vertex_traits::texcoord>(20).Write(ut::Vector<2>(1, 1));

	vertices.Get<vertex_traits::position>(21).Write(ut::Vector<3>(extent.X(), -extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(21).Write(ut::Vector<3>(0, 0, 1));
	vertices.Get<vertex_traits::texcoord>(21).Write(ut::Vector<2>(0, 1));

	vertices.Get<vertex_traits::position>(22).Write(ut::Vector<3>(extent.X(), extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(22).Write(ut::Vector<3>(0, 0, 1));
	vertices.Get<vertex_traits::texcoord>(22).Write(ut::Vector<2>(0, 0));

	vertices.Get<vertex_traits::position>(23).Write(ut::Vector<3>(-extent.X(), extent.Y(), extent.Z()));
	vertices.Get<vertex_traits::normal>(23).Write(ut::Vector<3>(0, 0, 1));
	vertices.Get<vertex_traits::texcoord>(23).Write(ut::Vector<2>(1, 0));

	// set indices
	geometry.index_count = 36;
	geometry.index_buffer.Resize(sizeof(ut::uint32) * geometry.index_count);
	ut::uint32* indices = reinterpret_cast<ut::uint32*>(geometry.index_buffer.GetAddress());
	indices[0] = 3;   indices[1] = 1;   indices[2] = 0;
	indices[3] = 2;   indices[4] = 1;   indices[5] = 3;
	indices[6] = 6;   indices[7] = 4;   indices[8] = 5;
	indices[9] = 7;   indices[10] = 4;  indices[11] = 6;
	indices[12] = 11; indices[13] = 9;  indices[14] = 8;
	indices[15] = 10; indices[16] = 9;  indices[17] = 11;
	indices[18] = 14; indices[19] = 12; indices[20] = 13;
	indices[21] = 15; indices[22] = 12; indices[23] = 14;
	indices[24] = 19; indices[25] = 17; indices[26] = 16;
	indices[27] = 18; indices[28] = 17; indices[29] = 19;
	indices[30] = 22; indices[31] = 20; indices[32] = 21;
	indices[33] = 23; indices[34] = 20; indices[35] = 22;

	// calculate tangents
	ComputeTangents(input_assembly, geometry);

	// apply position offset
	for (ut::uint32 i = 0; i < geometry.vertex_count; i++)
	{
		VertexReflector::Component vertex_position = vertices.Get<vertex_traits::position>(i);
		vertex_position.Write(vertex_position.Read<3, float>() + offset);
	}

	return geometry;
}

// Generates vertices for the spherical mesh.
// 	  @param vertex_format - desired vertex format of the final mesh.
//    @param polygon_mode - desired polygon mode of the final mesh.
// 	  @param offset - offset applied to all vertices.
//    @param radius - radius of the sphere.
// 	  @param segments - number of segments.
//    @return - ve::render::ResourceCreator<Mesh>::Geometry data object.
ResourceCreator<Mesh>::GeometryData ResourceCreator<Mesh>::GenSphereVertices(Mesh::VertexFormat vertex_format,
                                                                             Mesh::PolygonMode polygon_mode,
                                                                             const ut::Vector<3>& offset,
                                                                             float radius,
                                                                             ut::uint32 segment_count)
{
	ResourceCreator<Mesh>::GeometryData geometry;
	const InputAssemblyState input_assembly = Mesh::CreateIaState(vertex_format, Mesh::PolygonMode::triangle);

	// initialize vertices
	const ut::uint32 nv = segment_count;
	const ut::uint32 nh = segment_count;
	geometry.vertex_count = (nv + 1) * (nh + 1);
	geometry.vertex_buffer.Resize(input_assembly.vertex_stride * geometry.vertex_count);
	VertexReflector vertices(input_assembly, geometry.vertex_buffer.GetAddress());
	for (ut::uint32 i = 0; i < nv + 1; i++)
	{
		const float i_nv = static_cast<float>(i) / static_cast<float>(nv);
		const float AngleXZ = 2.0f * ut::Precision<float>::pi * i_nv;
		for (ut::uint32 j = 0; j < nh + 1; j++)
		{
			const float j_nh = static_cast<float>(j) / static_cast<float>(nh);
			const float AngleY = ut::Precision<float>::pi * j_nh;
			const ut::uint32 id = i * (nh + 1) + j;

			const ut::Vector<3> position(ut::Cos(AngleXZ) * ut::Sin(AngleY) * radius,
			                             ut::Cos(AngleY) * radius,
			                             ut::Sin(AngleXZ) * ut::Sin(AngleY) * radius);

			const ut::Vector<3> normal = position.Normalize();

			const ut::Vector<2> texcoord(i_nv, 1.0f - j_nh);

			vertices.Get<vertex_traits::position>(id).Write(position);
			vertices.Get<vertex_traits::normal>(id).Write(normal);
			vertices.Get<vertex_traits::texcoord>(id).Write(texcoord);
		}
	}

	// initialize indices
	geometry.index_count = nv * nh * 6;
	geometry.index_buffer.Resize(sizeof(ut::uint32) * geometry.index_count);
	ut::uint32* indices = reinterpret_cast<ut::uint32*>(geometry.index_buffer.GetAddress());
	ut::uint32 ind = 0;
	for (ut::uint32 i = 0; i < nv; i++)
	{
		for (ut::uint32 j = 0; j < nh; j++)
		{
			const ut::uint32 vi = (nh + 1) * i + j;
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

	// calculate tangents
	ComputeTangents(input_assembly, geometry);

	// apply position offset
	for (ut::uint32 i = 0; i < geometry.vertex_count; i++)
	{
		VertexReflector::Component vertex_position = vertices.Get<vertex_traits::position>(i);
		vertex_position.Write(vertex_position.Read<3, float>() + offset);
	}

	return geometry;
}

// Generates vertices for the torus mesh.
// 	  @param vertex_format - desired vertex format of the final mesh.
//    @param polygon_mode - desired polygon mode of the final mesh.
// 	  @param offset - offset applied to all vertices.
//    @param major_radius - major radius.
// 	  @param minor_radius - minor radius.
// 	  @param radial_segment_count - number of radial segments.
//    @param tubular_segment_count - number of tubular segments.
// 	  @param orientation - direction of the major radius plane's normal.
//    @return - ve::render::ResourceCreator<Mesh>::Geometry data object.
ResourceCreator<Mesh>::GeometryData ResourceCreator<Mesh>::GenTorusVertices(Mesh::VertexFormat vertex_format,
                                                                            Mesh::PolygonMode polygon_mode,
                                                                            const ut::Vector<3>& offset,
                                                                            float major_radius,
                                                                            float minor_radius,
                                                                            ut::uint32 radial_segment_count,
                                                                            ut::uint32 tubular_segment_count,
                                                                            AxisOrientation orientation)
{
	ResourceCreator<Mesh>::GeometryData geometry;
	const InputAssemblyState input_assembly = Mesh::CreateIaState(vertex_format, Mesh::PolygonMode::triangle);

	// allocate vertex buffer
	geometry.vertex_count = (radial_segment_count + 1) * (tubular_segment_count + 1);
	geometry.vertex_buffer.Resize(input_assembly.vertex_stride * geometry.vertex_count);
	VertexReflector vertices(input_assembly, geometry.vertex_buffer.GetAddress());

	// allocate index buffer
	geometry.index_count = radial_segment_count * tubular_segment_count * 6;
	geometry.index_buffer.Resize(sizeof(ut::uint32) * geometry.index_count);
	ut::uint32* indices = reinterpret_cast<ut::uint32*>(geometry.index_buffer.GetAddress());

	// calculate the orientation basis
	ut::byte basis_id[3];
	switch (orientation)
	{
		case AxisOrientation::x: basis_id[0] = 1; basis_id[1] = 2; basis_id[2] = 0; break;
		case AxisOrientation::y: basis_id[0] = 2; basis_id[1] = 0; basis_id[2] = 1; break;
		case AxisOrientation::z: basis_id[0] = 0; basis_id[1] = 1; basis_id[2] = 2; break;
	}

	// initialize vertices and indices in the same loop
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

			const ut::uint32 id = i * (tubular_segment_count + 1) + j;

			ut::Vector<3> position, normal;

			float* position_data = position.GetData();
			*(position_data + basis_id[0]) = (major_radius + minor_radius * cosV) * cosU;
			*(position_data + basis_id[1]) = (major_radius + minor_radius * cosV) * sinU;
			*(position_data + basis_id[2]) = minor_radius * sinV;

			float* normal_data = normal.GetData();
			*(normal_data + basis_id[0]) = cosV * cosU;
			*(normal_data + basis_id[1]) = cosV * sinU;
			*(normal_data + basis_id[2]) = sinV;

			const ut::Vector<2> texcoord(static_cast<float>(i) / static_cast<float>(radial_segment_count),
			                             static_cast<float>(j) / static_cast<float>(tubular_segment_count));

			vertices.Get<vertex_traits::position>(id).Write(position);
			vertices.Get<vertex_traits::normal>(id).Write(normal);
			vertices.Get<vertex_traits::texcoord>(id).Write(texcoord);

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

	// calculate tangents
	ComputeTangents(input_assembly, geometry);

	// apply position offset
	for (ut::uint32 i = 0; i < geometry.vertex_count; i++)
	{
		VertexReflector::Component vertex_position = vertices.Get<vertex_traits::position>(i);
		vertex_position.Write(vertex_position.Read<3, float>() + offset);
	}

	return geometry;
}

// Creates a generator prompt for a default surface material with solid color.
ut::String ResourceCreator<Mesh>::GenDefaultMaterialPrompt(const ut::Optional<ut::String>& color)
{
	ut::String material_prompt = Resource::GeneratorPrompt::skStarter;
	material_prompt += Material::Generator::skTypeSurface;

	if (color)
	{
		material_prompt += Resource::GeneratorPrompt::skSeparatorChr0;
		material_prompt += ut::String("c") +
		                   Resource::GeneratorPrompt::skValueSeparatorChr +
		                   color.Get();
	}

	return material_prompt;
}

// Creates a simple mesh resource with single subset from the provided
// index and vertex data.
ut::Result<RcRef<Mesh>, ut::Error> ResourceCreator<Mesh>::CreatePrimitive(ut::Optional<ut::String> name,
                                                                          Mesh::VertexFormat vertex_format,
                                                                          Mesh::PolygonMode polygon_mode,
                                                                          GeometryData geometry_data,
                                                                          const ut::Optional<ut::String>& material_prompt,
                                                                          const ut::Optional<ut::String>& color,
                                                                          Device& device,
                                                                          ResourceManager& rc_mgr)
{
	// create vertex buffer
	Buffer::Info vertex_buffer_info;
	vertex_buffer_info.type = Buffer::vertex;
	vertex_buffer_info.usage = render::memory::gpu_immutable;
	vertex_buffer_info.size = geometry_data.vertex_buffer.GetSize();
	vertex_buffer_info.stride = static_cast<ut::uint32>(vertex_buffer_info.size) / geometry_data.vertex_count;
	vertex_buffer_info.data = ut::Move(geometry_data.vertex_buffer);
	ut::Result<Buffer, ut::Error> vertex_buffer = device.CreateBuffer(ut::Move(vertex_buffer_info));
	if (!vertex_buffer)
	{
		return ut::MakeError(vertex_buffer.MoveAlt());
	}

	// create index buffer
	Buffer::Info index_buffer_info;
	index_buffer_info.type = Buffer::index;
	index_buffer_info.usage = render::memory::gpu_immutable;
	index_buffer_info.size = sizeof(ut::uint32) * geometry_data.index_count;
	index_buffer_info.stride = sizeof(ut::uint32);
	index_buffer_info.data = ut::Move(geometry_data.index_buffer);
	ut::Result<Buffer, ut::Error> index_buffer = device.CreateBuffer(ut::Move(index_buffer_info));
	if (!index_buffer)
	{
		return ut::MakeError(index_buffer.MoveAlt());
	}

	// initialize subsets
	ut::Array<Mesh::Subset> subsets;
	if (!material_prompt || material_prompt.Get() != "no")
	{
		// generate default material prompt if it wasn't set manually
		const ut::String final_material_prompt = material_prompt ?
		                                         material_prompt.Get() :
		                                         GenDefaultMaterialPrompt(color);

		// create material
		ut::Result<Material, ut::Error> material = Material::Generator::CreateFromPrompt(final_material_prompt,
			rc_mgr);
		if (!material)
		{
			return ut::MakeError(material.MoveAlt());
		}

		// initialize single subset
		Mesh::Subset subset;
		subset.index_offset = 0;
		subset.index_count = geometry_data.index_count;
		subset.material = material.Move();
		subsets.Add(ut::Move(subset));
	}

	// create mesh
	Mesh mesh(geometry_data.index_count / Mesh::GetPolygonVertexCount(polygon_mode),
	          geometry_data.vertex_count,
	          vertex_buffer.Move(),
	          index_buffer.Move(),
	          index_type_uint32,
	          vertex_format,
	          polygon_mode,
	          ut::Move(subsets));

	return rc_mgr.AddResource<Mesh>(ut::Move(mesh), ut::Move(name));
}

// Helper function to compute tangents.
void ResourceCreator<Mesh>::ComputeTangents(const InputAssemblyState& input_assembly,
                                            GeometryData& geometry_data)
{
	VertexReflector vertices(input_assembly, geometry_data.vertex_buffer.GetAddress());
	const ut::uint32* indices = reinterpret_cast<const ut::uint32*>(geometry_data.index_buffer.GetAddress());

	for (ut::uint32 i = 0; i < geometry_data.index_count; i = i + 3)
	{
		ut::uint32 vid[3] = { indices[i], indices[i + 1], indices[i + 2] };

		const ut::Vector<3> positions[3] =
		{
			vertices.Get<vertex_traits::position>(vid[0]).Read<3, float>(),
			vertices.Get<vertex_traits::position>(vid[1]).Read<3, float>(),
			vertices.Get<vertex_traits::position>(vid[2]).Read<3, float>()
		};

		const ut::Vector<2> texcoords[3] =
		{
			vertices.Get<vertex_traits::texcoord>(vid[0]).Read<2, float>(),
			vertices.Get<vertex_traits::texcoord>(vid[1]).Read<2, float>(),
			vertices.Get<vertex_traits::texcoord>(vid[2]).Read<2, float>(),
		};

		const ut::Vector<3> tangents[3] =
		{
			vertices.Get<vertex_traits::tangent>(vid[0]).Read<3, float>(),
			vertices.Get<vertex_traits::tangent>(vid[1]).Read<3, float>(),
			vertices.Get<vertex_traits::tangent>(vid[2]).Read<3, float>()
		};

		const ut::Vector<3> d1 = positions[2] - positions[0];
		const ut::Vector<3> d2 = positions[1] - positions[0];
		const ut::Vector<2> lt1(texcoords[1].X() - texcoords[0].X(),
		                        texcoords[1].Y() - texcoords[0].Y());
		const ut::Vector<2> lt2(texcoords[2].X() - texcoords[0].X(),
		                        texcoords[2].Y() - texcoords[0].Y());
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

		vertices.Get<vertex_traits::tangent>(vid[0]).Write(tangents[0] + tangent);
		vertices.Get<vertex_traits::tangent>(vid[1]).Write(tangents[1] + tangent);
		vertices.Get<vertex_traits::tangent>(vid[2]).Write(tangents[2] + tangent);
	}

	// normalize tangents
	for (size_t i = 0; i < geometry_data.vertex_count; i++)
	{
		VertexReflector::Component tangent = vertices.Get<vertex_traits::tangent>(i);
		tangent.Write(tangent.Read<3, float>().Normalize());
	}
}

// Creates a box from the provided generator prompt.
// Acceptable prompt attributes are:
//   'x': x-coordinate offset, must be a floating point number,
//        default is 0.
//   'y': y-coordinate offset, must be a floating point number,
//        default is 0.
//   'z': z-coordinate offset, must be a floating point number,
//        default is 0.
//   'w': width (X axis), must be a floating point number, default is 1.
//   'h': height (Y axis), must be a floating point number, default is 1.
//   'd': depth (Z axis), must be a floating point number, default is 1.
//   'm': material prompt, see Material::Generator::CreateFromPrompt()
//        for details, also the value 'no' is acceptable indicating the
//        mesh won't have any material at all.
//   'c': name of the color to be used if 'm' parameter wasn't provided,
//        see ResourceCreator<Map>::GenColorNameMap() for details.
//   'l': enables line (wireframe) mode, must be 'yes' or 'no'. Default is 'no'.
ut::Result<RcRef<Mesh>, ut::Error> ResourceCreator<Mesh>::CreateBox(const ut::String& name,
                                                                    const Resource::GeneratorPrompt::Attributes& attributes,
                                                                    Device& device,
                                                                    ResourceManager& rc_mgr)
{
	// primitive parameters
	ut::Vector<3> position(0.0f);
	ut::Vector<3> extent(1.0f);
	ut::Optional<ut::String> material_prompt;
	ut::Optional<ut::String> color;
	bool line_mode = false;

	// update parameters using generator prompt attributes
	for (const Resource::GeneratorPrompt::Attribute& attribute : attributes)
	{
		switch (attribute.type)
		{
			case 'x': position.X() = ut::Scan<float>(attribute.value); break;
			case 'y': position.Y() = ut::Scan<float>(attribute.value); break;
			case 'z': position.Z() = ut::Scan<float>(attribute.value); break;
			case 'w': extent.X() = ut::Scan<float>(attribute.value) * 0.5f; break;
			case 'h': extent.Y() = ut::Scan<float>(attribute.value) * 0.5f; break;
			case 'd': extent.Z() = ut::Scan<float>(attribute.value) * 0.5f; break;
			case 'm': material_prompt = attribute.value; break;
			case 'c': color = attribute.value; break;
			case 'l': line_mode = attribute.value == "yes"; break;
		}
	}	

	// set vertex format and polygon mode
	constexpr Mesh::VertexFormat vertex_format = Mesh::VertexFormat::pos3_texcoord2_normal3_tangent3_float;
	const Mesh::PolygonMode polygon_mode = line_mode ? Mesh::PolygonMode::triangle_wireframe :
	                                                   Mesh::PolygonMode::triangle;

	// generate vertices and indices
	InputAssemblyState input_assembly = Mesh::CreateIaState(vertex_format, Mesh::PolygonMode::triangle);
	GeometryData geometry_data = GenBoxVertices(vertex_format, polygon_mode, position, extent);

	// create final primitive
	return CreatePrimitive(ut::Move(name),
	                       vertex_format,
	                       polygon_mode,
	                       geometry_data,
	                       material_prompt,
	                       color,
	                       device,
	                       rc_mgr);
}

// Creates a spherical mesh from the provided generator prompt.
// Acceptable prompt attributes are:
//   'r': radius, must be a floating point number, default is 1.
//   's': number of segments, must be unsigned integer, default is 16.
//   'x': x-coordinate offset, must be a floating point number,
//        default is 0.
//   'y': y-coordinate offset, must be a floating point number,
//        default is 0.
//   'z': z-coordinate offset, must be a floating point number,
//        default is 0.
//   'm': material prompt, see Material::Generator::CreateFromPrompt()
//        for details, also the value 'no' is acceptable indicating the
//        mesh won't have any material at all.
//   'c': name of the color to be used if 'm' parameter wasn't provided,
//        see ResourceCreator<Map>::GenColorNameMap() for details.
//   'l': enables line (wireframe) mode, must be 'yes' or 'no'. Default is 'no'.
ut::Result<RcRef<Mesh>, ut::Error> ResourceCreator<Mesh>::CreateSphere(const ut::String& name,
                                                                       const Resource::GeneratorPrompt::Attributes& attributes,
                                                                       Device& device,
                                                                       ResourceManager& rc_mgr)
{
	// primitive parameters
	float radius = 1.0f;
	ut::uint32 segment_count = 16;
	ut::Vector<3> position(0.0f);
	ut::Optional<ut::String> material_prompt;
	ut::Optional<ut::String> color;
	bool line_mode = false;

	// update parameters using generator prompt attributes
	for (const Resource::GeneratorPrompt::Attribute& attribute : attributes)
	{
		switch (attribute.type)
		{
			case 'r': radius = ut::Scan<float>(attribute.value); break;
			case 's': segment_count = ut::Scan<ut::uint32>(attribute.value); break;
			case 'x': position.X() = ut::Scan<float>(attribute.value); break;
			case 'y': position.Y() = ut::Scan<float>(attribute.value); break;
			case 'z': position.Z() = ut::Scan<float>(attribute.value); break;
			case 'm': material_prompt = attribute.value; break;
			case 'c': color = attribute.value; break;
			case 'l': line_mode = attribute.value == "yes"; break;
		}
	}

	// set vertex format and polygon mode
	constexpr Mesh::VertexFormat vertex_format = Mesh::VertexFormat::pos3_texcoord2_normal3_tangent3_float;
	const Mesh::PolygonMode polygon_mode = line_mode ? Mesh::PolygonMode::triangle_wireframe :
	                                                   Mesh::PolygonMode::triangle;

	// generate vertices and indices
	GeometryData geometry_data = GenSphereVertices(vertex_format,
	                                               polygon_mode,
	                                               position,
	                                               radius,
	                                               segment_count);

	// create final primitive
	return CreatePrimitive(ut::Move(name),
	                       vertex_format,
	                       polygon_mode,
	                       geometry_data,
	                       material_prompt,
	                       color,
	                       device,
	                       rc_mgr);
}

// Creates a torus mesh from the provided generator prompt.
// Acceptable prompt attributes are:
//   'r': major radius, must be a floating point number, default is 1.
//   't': minor radius (thickness), must be a floating point number,
//        default is 0.5.
//   's': number of segments, must be an unsigned integer, default is 24.
//   'x': x-coordinate offset, must be a floating point number,
//        default is 0.
//   'y': y-coordinate offset, must be a floating point number,
//        default is 0.
//   'z': z-coordinate offset, must be a floating point number,
//        default is 0.
//   'o': orientation, valid values are 'x', 'y' and 'z' meaning the
//        normal's direction of the major radius' plane.
//   'm': material prompt, see Material::Generator::CreateFromPrompt()
//        for details, also the value 'no' is acceptable indicating the
//        mesh won't have any material at all.
//   'c': name of the color to be used if 'm' parameter wasn't provided,
//        see ResourceCreator<Map>::GenColorNameMap() for details.
//   'l': enables line (wireframe) mode, must be 'yes' or 'no'. Default is 'no'.
ut::Result<RcRef<Mesh>, ut::Error> ResourceCreator<Mesh>::CreateTorus(const ut::String& name,
                                                                      const Resource::GeneratorPrompt::Attributes& attributes,
                                                                      Device& device,
                                                                      ResourceManager& rc_mgr)
{
	// primitive parameters
	float radius = 1.0f;
	float thickness = 0.5f;
	ut::uint32 radial_segment_count = 24;
	ut::Vector<3> position(0.0f);
	ut::String orientation;
	ut::Optional<ut::String> material_prompt;
	ut::Optional<ut::String> color;
	bool line_mode = false;

	// update parameters using generator prompt attributes
	for (const Resource::GeneratorPrompt::Attribute& attribute : attributes)
	{
		switch (attribute.type)
		{
			case 'r': radius = ut::Scan<float>(attribute.value); break;
			case 't': thickness = ut::Scan<float>(attribute.value); break;
			case 's': radial_segment_count = ut::Scan<ut::uint32>(attribute.value); break;
			case 'x': position.X() = ut::Scan<float>(attribute.value); break;
			case 'y': position.Y() = ut::Scan<float>(attribute.value); break;
			case 'z': position.Z() = ut::Scan<float>(attribute.value); break;
			case 'o': orientation = attribute.value; break;
			case 'm': material_prompt = attribute.value; break;
			case 'c': color = attribute.value; break;
			case 'l': line_mode = attribute.value == "yes"; break;
		}
	}

	// validate the orientation
	if (orientation != "x" && orientation != "y" && orientation != "z")
	{
		orientation = "x";
	}

	// calculate the orientation basis
	AxisOrientation axis_orientation;
	switch (orientation[0])
	{
		case 'x': axis_orientation = AxisOrientation::x; break;
		case 'y': axis_orientation = AxisOrientation::y; break;
		case 'z': axis_orientation = AxisOrientation::z; break;
		default: axis_orientation = AxisOrientation::x;
	}

	// set vertex format and polygon mode
	constexpr Mesh::VertexFormat vertex_format = Mesh::VertexFormat::pos3_texcoord2_normal3_tangent3_float;
	const Mesh::PolygonMode polygon_mode = line_mode ? Mesh::PolygonMode::triangle_wireframe :
	                                                   Mesh::PolygonMode::triangle;

	// generate vertices and indices
	GeometryData geometry_data = GenTorusVertices(vertex_format,
	                                              polygon_mode,
	                                              position,
	                                              radius,
	                                              thickness,
	                                              radial_segment_count,
	                                              radial_segment_count * 2 / 3,
	                                              axis_orientation);

	// create final primitive
	return CreatePrimitive(ut::Move(name),
	                       vertex_format,
	                       polygon_mode,
	                       geometry_data,
	                       material_prompt,
	                       color,
	                       device,
	                       rc_mgr);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
