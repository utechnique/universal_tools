//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_resource.h"
#include "systems/render/engine/ve_render_material.h"
#include "systems/render/engine/ve_render_vertex_factory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// The ve::render::Mesh resource is a collection of vertices, indices and faces
// that defines the shape of a polyhedral object.
class Mesh : public Resource
{
public:
	// Allowed vertex formats.
	// The mesh resource supports a limited number of vertex formats to prevent
	// excessive pipeline production.
	enum VertexFormat
	{
		vertex_pos2_float,
		vertex_pos2_texcoord2_float,
		vertex_pos3_float,
		vertex_pos3_normal3_float,
		vertex_pos3_texcoord2_float,
		vertex_pos3_texcoord2_normal3_tangent3_float,
		vertex_pos3_texcoord2_normal3_tangent3_weights4_float_bones4_uint32,
		vertex_format_count
	};
	
	// Supported polygon rendering mode
	enum class PolygonMode
	{
		line,
		triangle,
		triangle_wireframe,
		count
	};

	// Instancing mode
	enum class Instancing
	{
		on,
		off
	};

	// Mesh::Subset is a part of the mesh with a separate material
	// and rendering options.
	struct Subset
	{
		Material material;

		// index offset in the index buffer if it exists
		// or a vertex offset in the vertex buffer otherwise
		ut::uint32 index_offset = 0;

		// number of indices if the index buffer exists
		// or a number of vertices otherwise
		ut::uint32 index_count = 0;
	};

	// Constructor.
	Mesh(ut::uint32 in_face_count,
	     ut::uint32 in_vertex_count,
		 Buffer in_vertex_buffer,
		 ut::Optional<Buffer> in_index_buffer,
		 IndexType in_index_type,
	     VertexFormat in_vertex_format,
	     PolygonMode in_polygon_mode,
	     ut::Array<Subset> in_subsets);

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const override;

	// Creates the input assembly state from the provided vertex format.
	static InputAssemblyState CreateIaState(VertexFormat vertex_format,
	                                        PolygonMode polygon_mode,
	                                        Instancing instancing);

	// Converts provided Mesh::PolygonMode value to corresponding
	// RasterizationState::PolygonMode value.
	static RasterizationState::PolygonMode GetRasterizerPolygonMode(PolygonMode polygon_mode);

	// Creates the input assembly state for the desired subset of this mesh.
	InputAssemblyState CreateIaState(Instancing instancing = Instancing::off) const;

	// Returns an array of shader macros for the specified vertex format.
	static ut::Array<Shader::MacroDefinition> GenerateVertexMacros(VertexFormat vertex_format,
	                                                               Instancing instancing = Instancing::off);

	// Number of vertices in one face.
	static constexpr ut::uint32 skPolygonVertices = 3;

	// Instance id is a per-instance vertex element to
	// identify current instance in shader.
	static const char* skInstanceIdSemantic;
	static constexpr pixel::Format skInstanceIdFormat = pixel::r32_uint;

	// Number of triangles.
	ut::uint32 face_count;

	// Number of vertices in the vertex buffer.
	ut::uint32 vertex_count;

	// Vertex and index buffers.
	Buffer vertex_buffer;
	ut::Optional<Buffer> index_buffer;

	// Indicates if indices are 16-bit or 32-bit.
	IndexType index_type;

	// The format of the vertices in the vertex buffer.
	VertexFormat vertex_format;

	// The polygon mode of this mesh.
	PolygonMode polygon_mode;

	// Subset groups, each has it's own material.
	ut::Array<Subset> subsets;
};

//----------------------------------------------------------------------------//
// Helper struct to obtain vertex type by the appropriate format id.
template<Mesh::VertexFormat format> struct MeshVertex;

template<> struct MeshVertex<Mesh::vertex_pos2_float>
{ typedef Vertex<float, 2> Type; };

template<> struct MeshVertex<Mesh::vertex_pos2_texcoord2_float>
{ typedef Vertex<float, 2, float, 2> Type; };

template<> struct MeshVertex<Mesh::vertex_pos3_float>
{ typedef Vertex<float, 3> Type; };

template<> struct MeshVertex<Mesh::vertex_pos3_normal3_float>
{ typedef Vertex<float, 3, void, 0, float, 3> Type; };

template<> struct MeshVertex<Mesh::vertex_pos3_texcoord2_float>
{ typedef Vertex<float, 3, float, 2> Type; };

template<> struct MeshVertex<Mesh::vertex_pos3_texcoord2_normal3_tangent3_float>
{ typedef Vertex<float, 3, float, 2, float, 3, float, 3> Type; };

template<> struct MeshVertex<Mesh::vertex_pos3_texcoord2_normal3_tangent3_weights4_float_bones4_uint32>
{ typedef Vertex<float, 3, float, 2, float, 3, float, 3, float, 4, ut::uint32, 4> Type; };

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//