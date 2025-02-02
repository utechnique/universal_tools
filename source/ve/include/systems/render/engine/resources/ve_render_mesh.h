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
	enum class VertexFormat
	{
		pos2_float,
		pos2_texcoord2_float,
		pos3_float,
		pos3_normal3_float,
		pos3_texcoord2_float,
		pos3_texcoord2_normal3_tangent3_float,
		pos3_texcoord2_normal3_tangent3_weights4_float_bones4_uint32,
		count
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
	Mesh(ut::uint32 in_polygon_count,
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
	                                        Instancing instancing = Instancing::off);

	// Converts provided Mesh::PolygonMode value to corresponding
	// RasterizationState::PolygonMode value.
	static RasterizationState::PolygonMode GetRasterizerPolygonMode(PolygonMode polygon_mode);

	// Creates the input assembly state for the desired subset of this mesh.
	InputAssemblyState CreateIaState(Instancing instancing = Instancing::off) const;

	// Returns an array of shader macros for the specified vertex format.
	static ut::Array<Shader::MacroDefinition> GenerateVertexMacros(VertexFormat vertex_format,
	                                                               Instancing instancing = Instancing::off);

	// Returns a number of vertices to render one polygon using the provided mode.
	static ut::uint32 GetPolygonVertexCount(PolygonMode mode);

	// Number of vertices in one face.
	static constexpr ut::uint32 skTriangleVertices = 3;
	static constexpr ut::uint32 skLineVertices = 2;

	// Instance id is a per-instance vertex element to
	// identify current instance in shader.
	static const char* skInstanceIdSemantic;
	static constexpr pixel::Format skInstanceIdFormat = pixel::r32_uint;

	// Number of polygons.
	ut::uint32 polygon_count;

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

template<> struct MeshVertex<Mesh::VertexFormat::pos2_float>
{ typedef Vertex<float, 2> Type; };

template<> struct MeshVertex<Mesh::VertexFormat::pos2_texcoord2_float>
{ typedef Vertex<float, 2, float, 2> Type; };

template<> struct MeshVertex<Mesh::VertexFormat::pos3_float>
{ typedef Vertex<float, 3> Type; };

template<> struct MeshVertex<Mesh::VertexFormat::pos3_normal3_float>
{ typedef Vertex<float, 3, void, 0, float, 3> Type; };

template<> struct MeshVertex<Mesh::VertexFormat::pos3_texcoord2_float>
{ typedef Vertex<float, 3, float, 2> Type; };

template<> struct MeshVertex<Mesh::VertexFormat::pos3_texcoord2_normal3_tangent3_float>
{ typedef Vertex<float, 3, float, 2, float, 3, float, 3> Type; };

template<> struct MeshVertex<Mesh::VertexFormat::pos3_texcoord2_normal3_tangent3_weights4_float_bones4_uint32>
{ typedef Vertex<float, 3, float, 2, float, 3, float, 3, float, 4, ut::uint32, 4> Type; };

//----------------------------------------------------------------------------//
// This is the specialized version of the ve::render::ResourceCreator template
// class for ve::render::Mesh resources. It's capable of generating primitives
// like box, sphere, torus, etc.
template<> class ResourceCreator<Mesh>
{
public:
	// Contains vertex and index buffers, serves as an intermediate stage in
	// the process of creating a mesh.
	struct GeometryData
	{
		ut::uint32 index_count = 0;
		ut::uint32 vertex_count = 0;
		ut::Array<ut::byte> vertex_buffer;
		ut::Array<ut::byte> index_buffer;
	};

	// Describes the orientation of an object.
	enum class AxisOrientation
	{
		x, y, z
	};

	// Constructor initializes a hashmap of generation functions in order to be
	// able quickly pick the correct one (instead of comparing all mesh names in
	// brute-force manner) when user calls Create() method.
	ResourceCreator(Device& device, ResourceManager& rc_mgr);

	// Creates a mesh according to the provided name, path or generator prompt.
	ut::Result<RcRef<Mesh>, ut::Error> Create(const ut::String& name);

	// Creates a 2D rectangle with vertex format
	// ve::render::Mesh::vertex_pos2_texcoord2_float.
	ut::Result<RcRef<Mesh>, ut::Error> CreateEyeSpaceRect(const ut::Vector<2>& position = ut::Vector<2>(0),
	                                                      const ut::Vector<2>& extent = ut::Vector<2>(1));

	// Generates vertices for the box mesh.
	// 	  @param vertex_format - desired vertex format of the final mesh.
	//    @param polygon_mode - desired polygon mode of the final mesh.
	// 	  @param offset - offset applied to all vertices.
	//    @param extent - extent of the box.
	//    @return - ve::render::ResourceCreator<Mesh>::Geometry data object.
	static GeometryData GenBoxVertices(Mesh::VertexFormat vertex_format,
	                                   Mesh::PolygonMode polygon_mode,
	                                   const ut::Vector<3>& offset,
	                                   const ut::Vector<3>& extent);

	// Generates vertices for the spherical mesh.
	// 	  @param vertex_format - desired vertex format of the final mesh.
	//    @param polygon_mode - desired polygon mode of the final mesh.
	// 	  @param offset - offset applied to all vertices.
	//    @param radius - radius of the sphere.
	// 	  @param segments - number of segments.
	//    @return - ve::render::ResourceCreator<Mesh>::Geometry data object.
	static GeometryData GenSphereVertices(Mesh::VertexFormat vertex_format,
	                                      Mesh::PolygonMode polygon_mode,
	                                      const ut::Vector<3>& offset,
	                                      float radius,
	                                      ut::uint32 segment_count);

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
	static GeometryData GenTorusVertices(Mesh::VertexFormat vertex_format,
	                                     Mesh::PolygonMode polygon_mode,
	                                     const ut::Vector<3>& offset,
	                                     float major_radius,
	                                     float minor_radius,
	                                     ut::uint32 radial_segment_count,
	                                     ut::uint32 tubular_segment_count,
	                                     AxisOrientation orientation);

	// Helper function to compute tangents.
	static void ComputeTangents(const InputAssemblyState& input_assembly,
	                            GeometryData& geometry_data);

	// The list of names acceptable for generating a mesh from a generator prompt.
	static const char *skTypeBox, *skTypeSphere, *skTypeTorus;

private:
	// ve::render::ResourceCreator<Mesh>::Generator is a signature of function
	// generating a mesh from generator prompt.
	using Generator = ut::Result<RcRef<Mesh>, ut::Error>(const ut::String&,
	                                                     const Resource::GeneratorPrompt::Attributes&,
	                                                     Device& device,
	                                                     ResourceManager& rc_mgr);

	// Creates a generator prompt for a default surface material with solid color.
	static ut::String GenDefaultMaterialPrompt(const ut::Optional<ut::String>& color);

	// Creates a simple mesh resource with single subset from the provided
	// index and vertex data.
	static ut::Result<RcRef<Mesh>, ut::Error> CreatePrimitive(ut::Optional<ut::String> name,
                                                              Mesh::VertexFormat vertex_format,
                                                              Mesh::PolygonMode polygon_mode,
                                                              GeometryData geometry_data,
                                                              const ut::Optional<ut::String>& material_prompt,
                                                              const ut::Optional<ut::String>& color,
                                                              Device& device,
                                                              ResourceManager& rc_mgr);

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
	static Generator CreateBox;

	// Creates a spherical mesh from the provided generator prompt.
	// Acceptable prompt attributes are:
	//   'r': radius, must be a floating point number, default is 1.
	//   's': number of segments, must be an unsigned integer, default is 16.
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
	static Generator CreateSphere;

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
	static Generator CreateTorus;

	// All generator functions are gathered in this hashmap to be quickly picked
	// using a mesh shape name as a key instead of comparing all possible shape
	// names one by one.
	ut::HashMap<ut::String, ut::Function<Generator> > generators;

	// ve::render::Device and ve::renderResourceManager objects are essential
	// for creating a mesh resource.
	Device& device;
	ResourceManager& rc_mgr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//