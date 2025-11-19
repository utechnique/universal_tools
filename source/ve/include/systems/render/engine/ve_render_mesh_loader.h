//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/resources/ve_render_mesh.h"
#include "systems/render/engine/ve_render_image_loader.h"
#include "ve_transform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Gltf is a class representing an interface to load intermediate
// GLTF data from file and export its parts like meshes, textures, materials to
// corresponding ve::render objects.
class Gltf
{
public:
	// Supported GLTF version.
	static const char* skSupportedVersion;

	// Metadata about the glTF asset.
	struct Asset
	{
		ut::String version;
		ut::Optional<ut::String> generator;
		ut::Optional<ut::String> min_version;
	};

	// The root nodes of a scene.
	struct Scene
	{
		ut::Optional<ut::String> name;
		ut::Array<ut::uint32> nodes;
	};

	// A node in the node hierarchy.
	struct Node
	{
		ut::Optional<ut::String> name;
		ut::Array<ut::uint32> children;
		ut::Optional<ut::Matrix<4, 4, float>> matrix;
		ut::Optional<ut::Quaternion<float>> rotation;
		ut::Optional<ut::Vector<3, float>> scale;
		ut::Optional<ut::Vector<3, float>> translation;
		ut::Array<float> weights;
		ut::Optional<ut::uint32> camera;
		ut::Optional<ut::uint32> skin;
		ut::Optional<ut::uint32> mesh;
	};

	// A typed view into a buffer view that contains raw binary data.
	struct Accessor
	{
		// Allowed component types.
		enum class ComponentType : ut::uint32
		{
			signed_byte = 5120,
			unsigned_byte = 5121,
			signed_short = 5122,
			unsigned_short = 5123,
			unsigned_int = 5125,
			signed_float = 5126,
		};

		// Sparse storage of accessor values that deviate from
		// their initialization value.
		struct Sparse
		{
			// An object pointing to a buffer view containing the indices
			// of deviating accessor values. The number of indices is equal
			// to Accessor::Sparse::count.
			struct Indices
			{
				ut::uint32 buffer_view;
				ut::uint32 component_type;
				ut::Optional<ut::uint32> byte_offset;
			};

			// An object pointing to a buffer view containing the deviating
			// accessor values.The number of elements is equal to
			// Accessor::Sparse::count. times number of components. The elements
			// have the same component type as the base accessor.
			struct Values
			{
				ut::uint32 buffer_view;
				ut::Optional<ut::uint32> byte_offset;
			};

			ut::uint32 count;
			Indices indices;
			Values values;
		};

		ComponentType component_type;
		ut::uint32 count;
		ut::String type;
		ut::Optional<ut::String> name;
		ut::Optional<ut::uint32> buffer_view;
		ut::Optional<ut::uint32> byte_offset;
		ut::Optional<bool> normalized;
		ut::Optional<ut::Array<float>> max_component_value;
		ut::Optional<ut::Array<float>> min_component_value;
		ut::Optional<Sparse> sparse;
	};

	// A set of primitives to be rendered. Its global transform is
	// defined by a node that references it.
	struct Mesh
	{
		// Geometry to be rendered with the given material.
		struct Primitive
		{
			// Allowed vertex attribute names.
			static const char* skPositionAttrName;
			static const char* skNormalAttrName;
			static const char* skTangentAttrName;
			static const char* skTexcoordAttrName;
			static const char* skColorAttrName;
			static const char* skJointsAttrName;
			static const char* skWeightsAttrName;

			// Allowed draw modes.
			enum class Mode : ut::uint32
			{
				points = 0,
				lines = 1,
				line_loop = 2,
				line_strip = 3,
				triangles = 4,
				triangle_strip = 5,
				triangle_fan = 6,
				default_value = triangles,
				max_value = triangle_fan
			};

			// A map where each key corresponds to a mesh attribute
			// semantic and each value is the index of the accessor
			// containing attribute’s data.
			typedef ut::Pair<ut::String, ut::uint32> Attribute;
			typedef ut::Array<Attribute> Attributes;

			Attributes attributes;
			ut::Array<Attributes> morph_targets;
			ut::Optional<ut::uint32> indices;
			ut::Optional<ut::uint32> material;
			ut::Optional<Mode> mode;
		};

		ut::Optional<ut::String> name;
		ut::Array<Primitive> primitives;
		ut::Array<float> weights;
	};

	// A buffer points to binary geometry, animation, or skins.
	struct Buffer
	{
		ut::uint32 byte_length;
		ut::Optional<ut::String> uri;
		ut::Optional<ut::String> name;
		ut::Array<ut::byte> data;
	};

	// A view into a buffer generally representing a subset of the buffer.
	struct BufferView
	{
		ut::uint32 buffer_id;
		ut::uint32 byte_length;
		ut::Optional<ut::String> name;
		ut::Optional<ut::uint32> byte_offset;
		ut::Optional<ut::uint32> byte_stride;
		ut::Optional<ut::uint32> target;
	};

	// Image data used to create a texture. Image MAY be referenced by
	// an URI (or IRI) or a buffer view index.
	struct Image
	{
		ut::Optional<ut::String> uri;
		ut::Optional<ut::uint32> buffer_view;
		ut::Optional<ut::String> mime_type;
		ut::Optional<ut::String> name;
	};

	// A texture and its sampler.
	struct Texture
	{
		ut::Optional<ut::uint32> sampler;
		ut::Optional<ut::uint32> source;
		ut::Optional<ut::String> name;
	};

	// Reference to a texture.
	struct TextureInfo
	{
		ut::uint32 index;
		ut::Optional<ut::uint32> texcoord;
	};

	// The material appearance of a primitive.
	struct Material
	{
		// A set of parameter values that are used to define the
		// metallic-roughness material model from Physically Based
		// Rendering (PBR) methodology.
		struct PbrMetallicRoughness
		{
			ut::Optional<ut::Vector<4, float>> base_color_factor;
			ut::Optional<float> metallic_factor;
			ut::Optional<float> roughness_factor;
			ut::Optional<TextureInfo> base_color_texture;
			ut::Optional<TextureInfo> metallic_roughness_texture;
		};

		// The tangent space normal texture. The texture encodes RGB
		// components with linear transfer function. Each texel represents
		// the XYZ components of a normal vector in tangent space. The normal
		// vectors use the convention +X is right and +Y is up. +Z points
		// toward the viewer.
		struct NormalTextureInfo
		{
			ut::uint32 index;
			ut::Optional<ut::uint32> texcoord;
			ut::Optional<float> scale;
		};

		// The occlusion texture. The occlusion values are linearly sampled
		// from the R channel. Higher values indicate areas that receive full
		// indirect lighting and lower values indicate no indirect lighting.
		struct OcclusionTextureInfo
		{
			ut::uint32 index;
			ut::Optional<ut::uint32> texcoord;
			ut::Optional<float> strength;
		};

		ut::Optional<ut::String> name;
		ut::Optional<ut::String> alpha_mode;
		ut::Optional<float> alpha_cutoff;
		ut::Optional<bool> double_sided;
		ut::Optional<ut::Vector<3, float>> emissive_factor;
		ut::Optional<TextureInfo> emissive_texture;
		ut::Optional<PbrMetallicRoughness> pbr_metallic_roughness;
		ut::Optional<NormalTextureInfo> normal_texture;
		ut::Optional<OcclusionTextureInfo> occlusion_texture;
	};

	// Texture sampler properties for filtering and wrapping modes.
	struct Sampler
	{
		ut::Optional<ut::uint32> mag_filter;
		ut::Optional<ut::uint32> min_filter;
		ut::Optional<ut::uint32> wrap_su;
		ut::Optional<ut::uint32> wrap_tv;
		ut::Optional<ut::String> name;
	};

	// Constructor, loads and initializes intermediate GLTF data from file.
	//    @path - a path to the file to be loaded.
	Gltf(const ut::String& path);

	// Constructs a new ve::render::Mesh object from one or all
	// meshes included in this GLTF file.
	//    @device - a reference to the render device to create GPU resources
	//              like vertex/index buffer.
	//    @rc_mgr - a reference to the resource manager to create new render
	//              resources like ve::Render::Map.
	//    @export_mesh_id - optional index of the mesh to be loaded; if this
	//                      parameter is empty, all meshes from the node
	//                      hierarchy will be exported into one ve::render::Mesh
	//                      object and transformed according to the owning nodes.
	//    @return - a new ve::render::Mesh object or ut::Error if failed.
	ut::Result<render::Mesh, ut::Error> ExportMesh(Device& device,
	                                               ResourceManager& rc_mgr,
	                                               ut::Optional<size_t> export_mesh_id = ut::Optional<size_t>()) const;

	// Constructs a new ve::render::Material object from a desired material
	// included in this GLTF file.
	//    @rc_mgr - a reference to the resource manager to create new render
	//              resources like ve::Render::Map.
	// 	  @img_loader - a reference to the image loader to load image data.
	//    @material_id - index of the desired material.
	//    @return - a new ve::render::Material object or ut::Error if failed.
	ut::Result<render::Material, ut::Error> ExportMaterial(ResourceManager& rc_mgr,
	                                                       const ImageLoader& img_loader,
	                                                       size_t material_id) const;

	// Constructs a new ve::render::Map object from a desired texture
	// included in this GLTF file.
	//    @rc_mgr - a reference to the resource manager to create a new map resource.
	// 	  @img_loader - a reference to the image loader to load image data.
	//    @texture_id - index of the desired texture.
	// 	  @srgb - indicates if the map must be stored in SRGB format.
	//    @return - a new ve::render::Material object or ut::Error if failed.
	ut::Result<RcRef<render::Map>, ut::Error> ExportTexture(ResourceManager& rc_mgr,
	                                                        const ImageLoader& img_loader,
	                                                        size_t texture_id,
	                                                        bool srgb = false) const;

	// The path to the directory containing this GLTF file. 
	ut::String root_dir;

	// The name of this GLTF file.
	ut::String filename;

	// GLTF json file.
	ut::JsonDoc json;

	// Deserialized GLTF data.
	Asset asset;
	ut::Optional<ut::uint32> scene_id;
	ut::Array<Scene> scenes;
	ut::Array<Node> nodes;
	ut::Array<Mesh> meshes;
	ut::Array<Accessor> accessors;
	ut::Array<Buffer> buffers;
	ut::Array<BufferView> buffer_views;
	ut::Array<Image> images;
	ut::Array<Sampler> samplers;
	ut::Array<Texture> textures;
	ut::Array<Material> materials;

private:
	// Reads data managed by the desired accessor.
	//    @param accessor_id - an ID of the desired accessor.
	//    @return - an array of vectorized data or ut::Error if failed.
	template<ut::uint32 dim, typename Scalar>
	ut::Result<ut::Array<ut::Vector<dim, Scalar>>,
	           ut::Error> ReadAccessorData(ut::uint32 accessor_id) const
	{
		ut::Array<ut::Vector<dim, Scalar>> data;

		if (accessor_id >= accessors.Count())
		{
			return ut::MakeError(ut::error::out_of_bounds, "Invalid accessor index.");
		}

		const Accessor& accessor = accessors[accessor_id];

		if (!accessor.buffer_view)
		{
			return ut::MakeError(ut::error::empty,
			                     "Unable to read accessor data from undefined buffer view.");
		}

		const ut::uint32 buffer_view_id = accessor.buffer_view.Get();
		if (buffer_view_id >= buffer_views.Count())
		{
			return ut::MakeError(ut::error::out_of_bounds, "Invalid buffer view index.");
		}

		const BufferView& buffer_view = buffer_views[buffer_view_id];

		if (buffer_view.buffer_id >= buffers.Count())
		{
			return ut::MakeError(ut::error::out_of_bounds, "Invalid buffer index.");
		}

		const Buffer& buffer = buffers[buffer_view.buffer_id];
		ut::uint32 offset = buffer_view.byte_offset ? buffer_view.byte_offset.Get() : 0;
		offset += accessor.byte_offset ? accessor.byte_offset.Get() : 0;
		const ut::uint32 buffer_length = buffer_view.byte_length;

		// get component count
		const ut::uint32 component_count = accessor.type == "SCALAR" ? 1 :
		                                   accessor.type == "VEC2" ? 2 :
		                                   accessor.type == "VEC3" ? 3 :
		                                   accessor.type == "VEC4" ? 4 :
		                                   accessor.type == "MAT2" ? 4 :
		                                   accessor.type == "MAT3" ? 9 :
		                                   accessor.type == "MAT4" ? 16 :
		                                   0;
		if (component_count == 0)
		{
			return ut::MakeError(ut::error::fail, "Invalid accessor type.");
		}

		// calculate component size and a size of one element
		const ut::TypeId dst_component_type = ut::Type<Scalar>::Id();
		const ut::uint32 dst_element_size = sizeof(ut::Vector<dim, Scalar>);
		const ut::uint32 dst_component_size = dst_element_size / dim;
		const ut::TypeId component_type = accessor.component_type == Accessor::ComponentType::signed_byte ? ut::Type<ut::int8>::Id() :
		                                  accessor.component_type == Accessor::ComponentType::unsigned_byte ? ut::Type<ut::byte>::Id() :
		                                  accessor.component_type == Accessor::ComponentType::signed_short ? ut::Type<ut::int16>::Id() :
		                                  accessor.component_type == Accessor::ComponentType::unsigned_short ? ut::Type<ut::uint16>::Id() :
		                                  accessor.component_type == Accessor::ComponentType::unsigned_int ? ut::Type<ut::uint32>::Id() :
		                                  ut::Type<float>::Id();
		const ut::uint32 component_size = accessor.component_type == Accessor::ComponentType::signed_byte ? 1 :
		                                  accessor.component_type == Accessor::ComponentType::unsigned_byte ? 1 :
		                                  accessor.component_type == Accessor::ComponentType::signed_short ? 2 :
		                                  accessor.component_type == Accessor::ComponentType::unsigned_short ? 2 :
		                                  4;
		const ut::uint32 element_size = component_size * component_count;

		// prepare binary stream for reading
		ut::BinaryStream stream;
		stream.SetBuffer(buffer.data);
		stream.MoveCursor(offset);

		// read elements one by one
		data.Resize(accessor.count);
		for (ut::uint32 element_id = 0; element_id < accessor.count; element_id++)
		{
			if (offset + element_size > buffer.data.GetSize())
			{
				return ut::MakeError(ut::error::out_of_bounds,
				                     "Got out of bounds while reading from buffer.");
			}

			for (ut::uint32 component_id = 0; component_id < component_count; component_id++)
			{
				ut::byte component_buffer[4]; // maximum 4 components

				constexpr ut::endianness::Order gltf_order = ut::endianness::Order::little;
				ut::Optional<ut::Error> read_error = ut::endianness::Read<gltf_order>(stream,
				                                                                      component_buffer,
				                                                                      component_size,
				                                                                      1);
				if (read_error)
				{
					return ut::MakeError(read_error.Move());
				}

				const ut::uint32 dst_offset = element_id * dst_element_size +
				                              dst_component_size * component_id;

				ut::Convert(component_buffer,
				            component_type,
				            reinterpret_cast<ut::byte*>(data.GetAddress()) + dst_offset,
				            dst_component_type);
			}

			offset += element_size;
		}

		return data;
	}

	// Reads GLTF primitive data (positions, normals, etc.) to array of vectors.
	//    @param primitive - a reference to the desired GLTF mesh primitive.
	//    @param attr_name - a name of the desired attribute (position, normal, etc.)
	//    @return - an array of vectors or ut::Error if failed.
	template<ut::uint32 dim, typename Scalar>
	ut::Result<ut::Array<ut::Vector<dim, Scalar>>,
	           ut::Error> ReadPrimitiveAttrData(const Mesh::Primitive& primitive,
	                                            const ut::String& attr_name) const
	{
		const auto attribute = ut::FindIf(primitive.attributes.Begin(),
		                                  primitive.attributes.End(),
		                                  [=](const auto& attribute) { return attribute.first == attr_name; });
		if (!attribute)
		{
			return ut::MakeError(ut::error::not_found);
		}

		return ReadAccessorData<dim, Scalar>(attribute.Get()->second);
	}

	// Generates ve::render::ResourceCreator<render::Mesh>::GeometryData object
	// from the given GLTF mesh primitive.
	//    @param primitive - a reference to the desired GLTF mesh primitive.
	//    @param transform - a reference to the ve::Transform object to apply
	//                       to the exported geometry.
	//    @return - a new ResourceCreator<render::Mesh>::GeometryData object or
	//              ut::Error if failed.
	ut::Result<ResourceCreator<render::Mesh>::GeometryData,
	           ut::Error> ExportMeshGeometry(const Mesh::Primitive& primitive,
	                                         const Transform& transform) const;
	
	// Applies @transform to the vector @v and converts it from right-handed
	// to left-handed coordinate system.
	//    @param v - a reference to the vector to be transformed.
	//    @param transform - a reference to the ve::Transform object.
	//    @return - a transformed vector.
	static ut::Vector<3, float> TransformVector(const ut::Vector<3, float>& v,
	                                            const Transform& transform);

	// Trnasforms the given GLTF mesh primitive mode
	// to the render::Mesh::PolygonMode value.
	static ut::Result<render::Mesh::PolygonMode,
	                  ut::Error> ConvertMeshPrimitiveMode(Mesh::Primitive::Mode mode);

	// Loads the desired GLTF file from the given path.
	static ut::Result<ut::JsonDoc, ut::Error> LoadJsonFile(const ut::String& path);

	// Loads a GLTF asset object from the given JSON file.
	static ut::Result<Asset, ut::Error> LoadAsset(const ut::JsonDoc& json);

	// Loads an array of GLTF scenes from the given JSON file.
	static ut::Result<ut::Array<Scene>, ut::Error> LoadScenes(const ut::JsonDoc& json);

	// Loads the index of a GLTF scene to display from the given JSON file.
	static ut::Optional<ut::uint32> LoadSceneId(const ut::JsonDoc& json);

	// Loads an array of GLTF nodes from the given JSON file.
	static ut::Result<ut::Array<Node>, ut::Error> LoadNodes(const ut::JsonDoc& json);

	// Loads an array of GLTF meshes from the given JSON file.
	static ut::Result<ut::Array<Mesh>, ut::Error> LoadMeshes(const ut::JsonDoc& json);

	// Loads an array of GLTF mesh primitives from the given JSON file.
	static ut::Result<ut::Array<Mesh::Primitive>,
	                  ut::Error> LoadMeshPrimitives(const ut::Tree<ut::text::Node>& json_node);

	// Loads an array of GLTF mesh primitive attributes from the given JSON file.
	static ut::Result<Mesh::Primitive::Attributes,
	                  ut::Error> LoadPrimitiveAttributes(const ut::Tree<ut::text::Node>& json_node);

	// Loads an array of GLTF accessors from the given JSON file.
	static ut::Result<ut::Array<Accessor>, ut::Error> LoadAccessors(const ut::JsonDoc& json);

	// Loads a GLTF accessor sparse object from the given JSON file.
	static ut::Result<Accessor::Sparse,
	                  ut::Error> LoadAccessorSparse(const ut::Tree<ut::text::Node>& json_node);

	// Loads an array of GLTF buffers from the given JSON file.
	static ut::Result<ut::Array<Buffer>,
	                  ut::Error> LoadBuffers(const ut::JsonDoc& json,
	                                         const ut::String& root_dir);

	// Loads binary data from the given GLTF URI link.
	static ut::Result<ut::Array<ut::byte>,
	                  ut::Error> LoadUriData(const ut::String& uri,
	                                         const ut::String& root_dir);

	// Loads an array of GLTF buffer views from the given JSON file.
	static ut::Result<ut::Array<BufferView>,
	                  ut::Error> LoadBufferViews(const ut::JsonDoc& json);

	// Loads an array of GLTF images from the given JSON file.
	static ut::Result<ut::Array<Image>,
	                  ut::Error> LoadImages(const ut::JsonDoc& json);

	// Loads an array of GLTF samplers from the given JSON file.
	static ut::Result<ut::Array<Sampler>,
	                  ut::Error> LoadSamplers(const ut::JsonDoc& json);

	// Loads an array of GLTF textures from the given JSON file.
	static ut::Result<ut::Array<Texture>,
	                  ut::Error> LoadTextures(const ut::JsonDoc& json);

	// Loads an array of GLTF materials from the given JSON file.
	static ut::Result<ut::Array<Material>,
	                  ut::Error> LoadMaterials(const ut::JsonDoc& json);

	// Loads the desired GLTF texture information from the given JSON file.
	static ut::Result<TextureInfo,
	                  ut::Error> LoadTextureInfo(const ut::Tree<ut::text::Node>& json_node);

	// Loads the desired GLTF metallic-roughness object from the given JSON file.
	static ut::Result<Material::PbrMetallicRoughness,
	                  ut::Error> LoadPbrMetallicRoughness(const ut::Tree<ut::text::Node>& json_node);

	// Loads the desired GLTF normal map information from the given JSON file.
	static ut::Result<Material::NormalTextureInfo,
	                  ut::Error> LoadNormalTextureInfo(const ut::Tree<ut::text::Node>& json_node);

	// Loads the desired GLTF occlusion map information from the given JSON file.
	static ut::Result<Material::OcclusionTextureInfo,
	                  ut::Error> LoadOcclusionTextureInfo(const ut::Tree<ut::text::Node>& json_node);
};

//----------------------------------------------------------------------------->
// ve::render::MeshLoader encapsulates loading a mesh from file.
class MeshLoader
{
public:
	// Constructor.
	MeshLoader(Device& device_ref, ResourceManager& rc_mgr_ref) noexcept;

	// Loads a mesh from a file.
	//    @param filename - path to the mesh file to be loaded.
	//    @param info - const reference to the ImageLoader::Info object.
	//    @return - new render::Image object or ut::Error if failed.
	ut::Result<Mesh, ut::Error> Load(const ut::String& filename);

private:
	Device& device;
	ResourceManager& rc_mgr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
