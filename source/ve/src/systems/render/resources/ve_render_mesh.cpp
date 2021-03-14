//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/resources/ve_render_mesh.h"
//----------------------------------------------------------------------------//
UT_REGISTER_TYPE(ve::render::Resource, ve::render::Mesh, "mesh")
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Instance id is a per-instance vertex element to
	// identify current instance in shader.
const char* Mesh::skInstanceIdSemantic = "INSTANCE_ID";

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
Mesh::Mesh(ut::uint32 in_face_count,
           ut::uint32 in_vertex_count,
           Buffer in_vertex_buffer,
           ut::Optional<Buffer> in_index_buffer,
           IndexType in_index_type,
           VertexFormat in_vertex_format,
           ut::Array<Subset> in_subsets) : face_count(in_face_count)
                                         , vertex_count(in_vertex_count)
                                         , vertex_buffer(ut::Move(in_vertex_buffer))
                                         , index_buffer(ut::Move(in_index_buffer))
                                         , index_type(in_index_type)
                                         , vertex_format(in_vertex_format)
                                         , subsets(ut::Move(in_subsets))
{
	input_assembly.topology = primitive::triangle_list;
	ut::Optional<ut::Error> init_error = MeshVertexHelper<vertex_format_count - 1>::InitializeIaState(vertex_format,
	                                                                                                  input_assembly);
	UT_ASSERT(!init_error);

	VertexElement instance_id(skInstanceIdSemantic, skInstanceIdFormat, 0);
	input_assembly_instancing = input_assembly;
	input_assembly_instancing.instance_stride = pixel::GetSize(skInstanceIdFormat);
	input_assembly_instancing.instance_elements.Add(ut::Move(instance_id));
}

// Identify() method must be implemented for the polymorphic types.
const ut::DynamicType& Mesh::Identify() const
{
	return ut::Identify(this);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
