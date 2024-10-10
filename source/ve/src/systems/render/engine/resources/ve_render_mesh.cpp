//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/resources/ve_render_mesh.h"
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
           PolygonMode in_polygon_mode,
           ut::Array<Subset> in_subsets) : face_count(in_face_count)
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

	ut::Optional<ut::Error> init_error = MeshVertexHelper<vertex_format_count - 1>::InitializeIaState(vertex_format,
	                                                                                                  ia_state);
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

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
