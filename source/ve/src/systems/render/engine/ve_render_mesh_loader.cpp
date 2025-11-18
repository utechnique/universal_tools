//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_mesh_loader.h"
#include "systems/render/engine/ve_render_rc_mgr.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Supported GLTF version.
const char* Gltf::skSupportedVersion = "2.0";

// Allowed vertex attribute names.
const char* Gltf::Mesh::Primitive::skPositionAttrName = "POSITION";
const char* Gltf::Mesh::Primitive::skNormalAttrName = "NORMAL";
const char* Gltf::Mesh::Primitive::skTangentAttrName = "TANGENT";
const char* Gltf::Mesh::Primitive::skTexcoordAttrName = "TEXCOORD_";
const char* Gltf::Mesh::Primitive::skColorAttrName = "COLOR_";
const char* Gltf::Mesh::Primitive::skJointsAttrName = "JOINTS_";
const char* Gltf::Mesh::Primitive::skWeightsAttrName = "WEIGHTS_";

// Constructor, loads and initializes intermediate GLTF data from file.
//    @path - a path to the file to be loaded.
Gltf::Gltf(const ut::String& path) : root_dir(path.GetIsolatedLocation())
                                   , json(LoadJsonFile(path).MoveOrThrow())
                                   , asset(LoadAsset(json).MoveOrThrow())
                                   , scene_id(LoadSceneId(json))
                                   , scenes(LoadScenes(json).MoveOrThrow())
                                   , nodes(LoadNodes(json).MoveOrThrow())
                                   , meshes(LoadMeshes(json).MoveOrThrow())
                                   , accessors(LoadAccessors(json).MoveOrThrow())
                                   , buffers(LoadBuffers(json, root_dir).MoveOrThrow())
                                   , buffer_views(LoadBufferViews(json).MoveOrThrow())
                                   , images(LoadImages(json).MoveOrThrow())
                                   , samplers(LoadSamplers(json).MoveOrThrow())
                                   , textures(LoadTextures(json).MoveOrThrow())
                                   , materials(LoadMaterials(json).MoveOrThrow())
{}

// Constructs a new ve::render::Mesh object from one or all
// meshes included in this GLTF file.
//    @device - a reference to the render device to create GPU resources
//              like vertex/index buffer, textures, etc.
//    @rc_mgr - a reference to the resource manager to create new render
//              resources like ve::Render::Map.
//    @export_mesh_id - optional index of the mesh to be loaded; if this
//                      parameter is empty, all meshes from the node
//                      hierarchy will be exported into one ve::render::Mesh
//                      object and transformed according to the owning nodes.
//    @return - a new ve::render::Mesh object or ut::Error if failed.
ut::Result<render::Mesh, ut::Error> Gltf::ExportMesh(Device& device,
                                                     ResourceManager& rc_mgr,
                                                     ut::Optional<size_t> export_mesh_id) const
{
	// decide whether to export only one mesh or the whole node hierarchy
	bool apply_node_transform = false;
	ut::Array<size_t> mesh_export_indices;
	if (export_mesh_id)
	{
		mesh_export_indices.Add(export_mesh_id.Get());
	}
	else
	{
		apply_node_transform = true;
		const size_t mesh_count = meshes.Count();
		for (size_t mesh_iter = 0; mesh_iter < mesh_count; mesh_iter++)
		{
			mesh_export_indices.Add(mesh_iter);
		}
	}

	// a mesh can have primitives with different vertex format, we must
	// batch chunks of geometry data (vertices and indices) of the same format
	// in a set of vertex and index buffers
	ut::HashMap<render::Mesh::VertexFormat, render::Buffer::Info> vertex_data;
	ut::HashMap<render::Mesh::VertexFormat, render::Buffer::Info> index_data;

	// each gltf primitive must be converted to a ve::render::Mesh::Subset object
	struct SubsetIaInfo
	{
		render::Mesh::VertexFormat vertex_format;
		bool has_indices;
	};
	ut::Array<SubsetIaInfo> subset_ia_info;
	ut::Array<render::Mesh::Subset> subsets;

	// iterate over the meshes to be exported and build an array
	// of ve::render::Mesh::Subset objects to finally initialize a
	// ve::render::Mesh object
	for (const size_t mesh_id : mesh_export_indices)
	{
		// get the desired mesh from the mesh array
		if (mesh_id >= meshes.Count())
		{
			ut::String error_desc("Unable to export mesh #");
			error_desc += ut::Print(mesh_id);
			error_desc += " because loaded GLTF file has only ";
			error_desc += ut::Print(meshes.Count());
			error_desc += " meshes.";
			return ut::MakeError(ut::error::out_of_bounds, ut::Move(error_desc));
		}
		const Mesh& mesh = meshes[mesh_id];

		// extract transform from the corresponding node
		Transform transform;
		if (apply_node_transform)
		{
			auto find_node_result = ut::FindIf(nodes.Begin(), nodes.End(),
			                                   [=](const Node& n) {
			                                        return n.mesh.HasValue() &&
			                                               n.mesh.Get() == mesh_id;
			                                    });
			if (find_node_result)
			{
				const Node& node = *find_node_result.Get();

				if (node.rotation)
				{
					transform.rotation = node.rotation.Get();
				}

				if (node.translation)
				{
					transform.translation = node.translation.Get();
				}

				if (node.scale)
				{
					transform.scale = node.scale.Get();
				}
			}
		}

		// iterate primitives and convert them into a render::Mesh::Subset object
		for (const Mesh::Primitive& primitive : mesh.primitives)
		{
			// geometry data encapsulates a cpu representation of
			// the vertex/index buffer for the given mesh primitive
			ut::Result<ResourceCreator<render::Mesh>::GeometryData,
			                           ut::Error> geometry_result = ExportMeshGeometry(primitive,
			                                                                           transform);
			if (!geometry_result)
			{
				return ut::MakeError(geometry_result.MoveAlt());
			}
			ResourceCreator<render::Mesh>::GeometryData& geometry = geometry_result.Get();

			// accumulate vertices of the same format in one buffer
			ut::uint32 vertex_id_offset = 0;
			ut::Optional<render::Buffer::Info&> vertex_find_result = vertex_data.Find(geometry.vertex_format);
			if (vertex_find_result)
			{
				vertex_id_offset = static_cast<ut::uint32>(vertex_find_result->data.GetSize() /
				                                           geometry.input_assembly.vertex_stride);
				vertex_find_result->size += geometry.vertex_buffer.GetSize();
				vertex_find_result->data += ut::Move(geometry.vertex_buffer);
			}
			else
			{
				render::Buffer::Info buffer_info;
				buffer_info.type = render::Buffer::Type::vertex;
				buffer_info.usage = render::memory::Usage::gpu_immutable;
				buffer_info.size = geometry.vertex_buffer.GetSize();
				buffer_info.stride = geometry.input_assembly.vertex_stride;
				buffer_info.data = ut::Move(geometry.vertex_buffer);

				vertex_data.Insert(geometry.vertex_format, ut::Move(buffer_info));
			}

			// apply vertex index offset
			const bool has_indices = geometry.index_count > 0;
			if (has_indices)
			{
				ut::byte* index_ptr = geometry.index_buffer.GetAddress();
				for (ut::uint32 index_iter = 0; index_iter < geometry.index_count; index_iter++)
				{
					render::Mesh::IndexType* index = reinterpret_cast<render::Mesh::IndexType*>(index_ptr) +
					                                 index_iter;
					*index += vertex_id_offset;
				}
			}

			// accumulate indices in one buffer
			ut::uint32 index_offset = 0;
			ut::Optional<render::Buffer::Info&> index_find_result = index_data.Find(geometry.vertex_format);
			if (index_find_result)
			{
				index_offset = static_cast<ut::uint32>(index_find_result->size /
				                                       sizeof(render::Mesh::IndexType));
				index_find_result->size += geometry.index_buffer.GetSize();
				index_find_result->data += ut::Move(geometry.index_buffer);
			}
			else
			{
				render::Buffer::Info buffer_info;
				buffer_info.type = render::Buffer::Type::index;
				buffer_info.usage = render::memory::Usage::gpu_immutable;
				buffer_info.size = geometry.index_buffer.GetSize();
				buffer_info.stride = sizeof(render::Mesh::IndexType);
				buffer_info.data = ut::Move(geometry.index_buffer);

				index_data.Insert(geometry.vertex_format, ut::Move(buffer_info));
			}
			
			// create a material
			ut::Result<render::Material, ut::Error> material = primitive.material ?
				ExportMaterial(rc_mgr, primitive.material.Get()) :
				render::Material::Generator::CreateChecker(rc_mgr);
			if (!material)
			{
				return ut::MakeError(material.MoveAlt());
			}

			// initialize a subset
			render::Mesh::Subset subset;
			subset.material = material.Move();
			subset.polygon_mode = geometry.polygon_mode;
			subset.offset = has_indices ? index_offset :
			                              vertex_id_offset;
			subset.count = has_indices ? geometry.index_count :
			                             geometry.vertex_count;
			subsets.Add(ut::Move(subset));

			subset_ia_info.Add(SubsetIaInfo{ geometry.vertex_format, has_indices });
		}
	}

	// create vertex GPU buffers
	ut::HashMap<render::Mesh::VertexFormat,
	            ut::SharedPtr<render::Mesh::VertexBuffer,
	                          render::Mesh::Subset::skThreadSafety>> vertex_buffers;
	for (ut::Pair<const render::Mesh::VertexFormat,
	              render::Buffer::Info>& vertices : vertex_data)
	{
		ut::Result<render::Buffer,
		           ut::Error> vertex_buffer = device.CreateBuffer(ut::Move(vertices.second));
		if (!vertex_buffer)
		{
			return ut::MakeError(vertex_buffer.MoveAlt());
		}

		vertex_buffers.Insert(vertices.GetFirst(),
		                      ut::MakeUnsafeShared<render::Mesh::VertexBuffer>(vertex_buffer.Move(),
		                                                                       vertices.GetFirst()));
	}

	// create index GPU buffers
	ut::HashMap<render::Mesh::VertexFormat,
	            ut::SharedPtr<render::Mesh::IndexBuffer,
	                          render::Mesh::Subset::skThreadSafety>> index_buffers;
	for (ut::Pair<const render::Mesh::VertexFormat,
	              render::Buffer::Info>& indices : index_data)
	{
		ut::Result<render::Buffer,
		           ut::Error> index_buffer = device.CreateBuffer(ut::Move(indices.second));
		if (!index_buffer)
		{
			return ut::MakeError(index_buffer.MoveAlt());
		}

		index_buffers.Insert(indices.GetFirst(),
		                     ut::MakeUnsafeShared<render::Mesh::IndexBuffer>(index_buffer.Move(),
		                                                                     render::Mesh::index_format));
	}

	// bind a vertex/index buffer according to the subset's geometry format
	for (ut::uint32 subset_id = 0; subset_id < subsets.Count(); subset_id++)
	{
		const SubsetIaInfo& ia_info = subset_ia_info[subset_id];

		subsets[subset_id].vertex_buffer = vertex_buffers.Find(ia_info.vertex_format).Move();

		if (ia_info.has_indices)
		{
			subsets[subset_id].index_buffer = index_buffers.Find(ia_info.vertex_format).Move();
		}
	}

	// success
	return render::Mesh(ut::Move(subsets));
}

// Constructs a new ve::render::Material object from a desired material
// included in this GLTF file.
//    @rc_mgr - a reference to the resource manager to create new render
//              resources like ve::Render::Map.
//    @material_id - index of the desired material.
//    @return - a new ve::render::Material object or ut::Error if failed.
ut::Result<render::Material, ut::Error> Gltf::ExportMaterial(ResourceManager& rc_mgr,
                                                             size_t material_id) const
{
	if (material_id >= materials.Count())
	{
		return ut::MakeError(ut::error::out_of_bounds, "Invalid material ID.");
	}

	const Material& gltf_material = materials[material_id];

	render::Material material;

	// normal
	if (gltf_material.normal_texture)
	{
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = ExportTexture(rc_mgr,
		                                                 gltf_material.normal_texture->index);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.normal = map_result.Move();
	}
	else
	{
		const ut::String prompt = render::Material::Generator::
			CreateDefaultNormalPrompt();
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = rc_mgr.Acquire<render::Map>(prompt);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.normal = map_result.Move();
	}

	// base color
	if (gltf_material.pbr_metallic_roughness &&
	    gltf_material.pbr_metallic_roughness->base_color_texture)
	{
		const Material::PbrMetallicRoughness& pbr = gltf_material.pbr_metallic_roughness.Get();
		const bool srgb_format = true;
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = ExportTexture(rc_mgr,
		                                                 pbr.base_color_texture->index,
		                                                 srgb_format);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.base_color = map_result.Move();
	}
	else
	{
		const ut::String prompt = render::Material::Generator::
			CreateDefaultBaseColorPrompt();
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = rc_mgr.Acquire<render::Map>(prompt);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.base_color = map_result.Move();
	}

	// metallic-roughness
	if (gltf_material.pbr_metallic_roughness &&
		gltf_material.pbr_metallic_roughness->metallic_roughness_texture)
	{
		const Material::PbrMetallicRoughness& pbr = gltf_material.pbr_metallic_roughness.Get();
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = ExportTexture(rc_mgr,
		                                                 pbr.metallic_roughness_texture->index);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.metallic_roughness = map_result.Move();
	}
	else
	{
		const ut::String prompt = render::Material::Generator::
			CreateDefaultMetallicRoughnessPrompt();
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = rc_mgr.Acquire<render::Map>(prompt);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.metallic_roughness = map_result.Move();
	}

	// occlusion
	if (gltf_material.occlusion_texture)
	{
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = ExportTexture(rc_mgr,
		                                                 gltf_material.occlusion_texture->index);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.occlusion = map_result.Move();
	}
	else
	{
		const ut::String prompt = render::Material::Generator::
			CreateDefaultOcclusionPrompt();
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = rc_mgr.Acquire<render::Map>(prompt);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.occlusion = map_result.Move();
	}

	// emissive
	if (gltf_material.emissive_texture)
	{
		const bool srgb_format = true;
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = ExportTexture(rc_mgr,
		                                                 gltf_material.emissive_texture->index,
		                                                 srgb_format);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.emissive = map_result.Move();
	}
	else
	{
		const ut::String prompt = render::Material::Generator::
			CreateDefaultEmissivePrompt();
		ut::Result<RcRef<render::Map>,
		           ut::Error> map_result = rc_mgr.Acquire<render::Map>(prompt);
		if (!map_result)
		{
			return ut::MakeError(map_result.MoveAlt());
		}

		material.emissive = map_result.Move();
	}

	// alpha
	if (gltf_material.alpha_mode)
	{
		if (gltf_material.alpha_mode.Get() == "MASK")
		{
			material.alpha = render::Material::Alpha::masked;
		}
		else if (gltf_material.alpha_mode.Get() == "BLEND")
		{
			material.alpha = render::Material::Alpha::transparent;
		}
	}

	// double-sided
	if (gltf_material.double_sided)
	{
		material.double_sided = gltf_material.double_sided.Get();
	}

	return material;
}

// Constructs a new ve::render::Map object from a desired texture
// included in this GLTF file.
//    @rc_mgr - a reference to the resource manager to create a new map.
//    @texture_id - index of the desired texture.
// 	  @srgb - indicates if the map must be stored in SRGB format.
//    @return - a new ve::render::Material object or ut::Error if failed.
ut::Result<RcRef<render::Map>, ut::Error> Gltf::ExportTexture(ResourceManager& rc_mgr,
                                                              size_t texture_id,
                                                              bool srgb) const
{
	if (texture_id >= textures.Count())
	{
		return ut::MakeError(ut::error::out_of_bounds, "Invalid texture ID.");
	}

	const Texture& texture = textures[texture_id];
	if (!texture.source)
	{
		return ut::MakeError(ut::error::fail, "Texture source is absent.");
	}

	if (texture.source.Get() >= images.Count())
	{
		return ut::MakeError(ut::error::out_of_bounds, "Invalid image ID.");
	}

	const Image& img = images[texture.source.Get()];
	
	if (!img.uri)
	{
		return ut::MakeError(ut::error::out_of_bounds, "Image URI is absent.");
	}

	ut::String path;
	if (srgb)
	{
		path += ResourceCreator<Map>::skSrgbFileLoadPrefix;
	}

	path += root_dir + img.uri.Get();

	return rc_mgr.Acquire<render::Map>(path);
}

// Generates ve::render::ResourceCreator<render::Mesh>::GeometryData object
// from the given GLTF mesh primitive.
//    @param primitive - a reference to the desired GLTF mesh primitive.
//    @param transform - a reference to the ve::Transform object to apply
//                       to the exported geometry.
//    @return - a new ResourceCreator<render::Mesh>::GeometryData object or
//              ut::Error if failed.
ut::Result<ResourceCreator<render::Mesh>::GeometryData,
           ut::Error> Gltf::ExportMeshGeometry(const Mesh::Primitive& primitive,
                                               const Transform& transform) const
{
	ResourceCreator<render::Mesh>::GeometryData geometry_data;

	// names of the indexed (0..N) attributes
	const ut::String texcoord0_attr_name = ut::String(Gltf::Mesh::Primitive::skTexcoordAttrName) + "0";
	const ut::String weights0_attr_name = ut::String(Gltf::Mesh::Primitive::skWeightsAttrName) + "0";
	const ut::String joints0_attr_name = ut::String(Gltf::Mesh::Primitive::skJointsAttrName) + "0";

	// read indices
	if (primitive.indices)
	{
		ut::Result<ut::Array<ut::Vector<1, render::Mesh::IndexType>>, ut::Error> indices =
			ut::MakeError(ut::error::not_found);
		indices = ReadAccessorData<1, render::Mesh::IndexType>(primitive.indices.Get());
		if (!indices)
		{
			return ut::MakeError(indices.MoveAlt());
		}

		geometry_data.index_count = static_cast<ut::uint32>(indices->Count());
		geometry_data.index_buffer.Resize(indices->GetSize());
		ut::memory::Copy(geometry_data.index_buffer.GetAddress(),
		                 indices->GetAddress(),
		                 indices->GetSize());
	}

	// read positions
	auto positions = ReadPrimitiveAttrData<3, float>(primitive,
	                                                 Gltf::Mesh::Primitive::skPositionAttrName);
	if (!positions)
	{
		return positions.GetAlt().GetCode() == ut::error::not_found ?
			ut::MakeError(ut::error::fail, "Position attribute is absent.") :
			ut::MakeError(positions.MoveAlt());
	}
	geometry_data.vertex_count = static_cast<ut::uint32>(positions->Count());

	// read normals
	auto normals = ReadPrimitiveAttrData<3, float>(primitive,
	                                               Gltf::Mesh::Primitive::skNormalAttrName);
	if (!normals && normals.GetAlt().GetCode() != ut::error::not_found)
	{
		return ut::MakeError(normals.MoveAlt());
	}
	else if (normals && normals->Count() != geometry_data.vertex_count)
	{
		return ut::MakeError(ut::error::fail, "Invalid number of normals.");
	}

	// read tangents
	auto tangents = ReadPrimitiveAttrData<3, float>(primitive,
	                                                Gltf::Mesh::Primitive::skTangentAttrName);
	if (!tangents && tangents.GetAlt().GetCode() != ut::error::not_found)
	{
		return ut::MakeError(tangents.MoveAlt());
	}
	else if (tangents && tangents->Count() != geometry_data.vertex_count)
	{
		return ut::MakeError(ut::error::fail, "Invalid number of tangents.");
	}

	// read texcoord0
	auto texcoords = ReadPrimitiveAttrData<2, float>(primitive, texcoord0_attr_name);
	if (!texcoords && texcoords.GetAlt().GetCode() != ut::error::not_found)
	{
		return ut::MakeError(texcoords.MoveAlt());
	}
	else if (texcoords && texcoords->Count() != geometry_data.vertex_count)
	{
		return ut::MakeError(ut::error::fail, "Invalid number of texcoords.");
	}

	// read weights
	auto weights = ReadPrimitiveAttrData<4, float>(primitive, weights0_attr_name);
	if (!weights && weights.GetAlt().GetCode() != ut::error::not_found)
	{
		return ut::MakeError(weights.MoveAlt());
	}
	else if (weights && weights->Count() != geometry_data.vertex_count)
	{
		return ut::MakeError(ut::error::fail, "Invalid number of weights.");
	}

	// read bones
	auto bones = ReadPrimitiveAttrData<4, ut::uint32>(primitive, joints0_attr_name);
	if (!bones && bones.GetAlt().GetCode() != ut::error::not_found)
	{
		return ut::MakeError(bones.MoveAlt());
	}
	else if (bones && bones->Count() != geometry_data.vertex_count)
	{
		return ut::MakeError(ut::error::fail, "Invalid number of bones.");
	}

	// polygon mode
	const Mesh::Primitive::Mode mode = primitive.mode ? primitive.mode.Get() :
	                                   Mesh::Primitive::Mode::default_value;
	ut::Result<render::Mesh::PolygonMode, ut::Error> polygon_mode = ConvertMeshPrimitiveMode(mode);
	if (!polygon_mode)
	{
		return ut::MakeError(polygon_mode.MoveAlt());
	}
	geometry_data.polygon_mode = polygon_mode.Get();

	// vertex format
	if (normals && texcoords && weights && bones)
	{
		geometry_data.vertex_format = render::Mesh::VertexFormat::pos3_texcoord2_normal3_tangent3_weights4_float_bones4_uint32;
	}
	else if (normals && texcoords)
	{
		geometry_data.vertex_format = render::Mesh::VertexFormat::pos3_texcoord2_normal3_tangent3_float;
	}
	else if (normals)
	{
		geometry_data.vertex_format = render::Mesh::VertexFormat::pos3_normal3_float;
	}
	else
	{
		return ut::MakeError(ut::error::not_supported, "Unsupported vertex format.");
	}
	geometry_data.input_assembly = render::Mesh::CreateIaState(geometry_data.vertex_format,
	                                                           geometry_data.polygon_mode);

	// allocate vertex buffer cpu memory and update vertex elements
	geometry_data.vertex_buffer.Resize(geometry_data.input_assembly.vertex_stride *
	                                   geometry_data.vertex_count);
	VertexReflector vertices(geometry_data.input_assembly,
	                         geometry_data.vertex_buffer.GetAddress());
	for (ut::uint32 vertex_id = 0; vertex_id < geometry_data.vertex_count; vertex_id++)
	{
		vertices.Get<vertex_traits::position>(vertex_id).Write(TransformVector(positions->operator[](vertex_id), transform));

		if (normals)
		{
			vertices.Get<vertex_traits::normal>(vertex_id).Write(TransformVector(normals->operator[](vertex_id), transform));
		}

		if (tangents)
		{
			vertices.Get<vertex_traits::tangent>(vertex_id).Write(TransformVector(tangents->operator[](vertex_id), transform));
		}

		if (texcoords)
		{
			vertices.Get<vertex_traits::texcoord>(vertex_id).Write(texcoords->operator[](vertex_id));
		}

		if (weights && bones)
		{
			vertices.Get<vertex_traits::bone_weight>(vertex_id).Write(weights->operator[](vertex_id));
			vertices.Get<vertex_traits::bone_id>(vertex_id).Write(bones->operator[](vertex_id));
		}
	}

	// compute tangents
	const char* tangent_semantic_name = vertex_traits::Component<vertex_traits::tangent>::GetName();
	auto has_tangents = ut::FindIf(geometry_data.input_assembly.elements.Begin(),
	                               geometry_data.input_assembly.elements.End(),
	                               [=](const VertexElement& e) { return e.semantic_name == tangent_semantic_name; });
	if (has_tangents && !tangents)
	{
		ResourceCreator<render::Mesh>::ComputeTangents(geometry_data.input_assembly,
		                                               geometry_data);
	}

	// reverse winding order (gltf does it backwards)
	if (primitive.indices &&
	    geometry_data.polygon_mode == render::Mesh::PolygonMode::triangle ||
	    geometry_data.polygon_mode == render::Mesh::PolygonMode::triangle_wireframe)
	{
		ut::byte* index_buffer_ptr = geometry_data.index_buffer.GetAddress();
		if (geometry_data.index_count % 3 != 0)
		{
			return ut::MakeError(ut::error::fail, "Invalid number of indices.");
		}

		const ut::uint32 face_count = geometry_data.index_count / 3;
		for (ut::uint32 face_iter = 0; face_iter < face_count; face_iter++)
		{
			render::Mesh::IndexType* index = reinterpret_cast<render::Mesh::IndexType*>(index_buffer_ptr) + face_iter * 3;
			const render::Mesh::IndexType temp_index = index[1];
			index[1] = index[2];
			index[2] = temp_index;
		}
	}

	return geometry_data;
}

// Applies @transform to the vector @v and converts it from right-handed
// to left-handed coordinate system.
//    @param v - a reference to the vector to be transformed.
//    @param transform - a reference to the ve::Transform object.
//    @return - a transformed vector.
ut::Vector<3, float> Gltf::TransformVector(const ut::Vector<3, float>& v,
                                           const Transform& transform)
{
	ut::Vector<3, float> out = transform.rotation.Rotate(v);
	out += transform.translation;
	out = out.ElementWise() * transform.scale;
	out.X() = -out.X();
	return out;
}

// Trnasforms the given GLTF mesh primitive mode
// to the render::Mesh::PolygonMode value.
ut::Result<render::Mesh::PolygonMode,
           ut::Error> Gltf::ConvertMeshPrimitiveMode(Mesh::Primitive::Mode mode)
{
	switch (mode)
	{
		case Mesh::Primitive::Mode::lines: return render::Mesh::PolygonMode::line;
		case Mesh::Primitive::Mode::triangles: return render::Mesh::PolygonMode::triangle;
	}

	return ut::MakeError(ut::error::not_supported);
}

// Loads the desired GLTF file from the given path.
ut::Result<ut::JsonDoc, ut::Error> Gltf::LoadJsonFile(const ut::String& path)
{
	ut::JsonDoc json;

	try
	{
		ut::File file(path, ut::File::Access::read);
		file >> json;
	}
	catch (ut::Error error)
	{
		return ut::MakeError(ut::Move(error));
	}

	return json;
}

// Loads the GLTF asset object from the given JSON file.
ut::Result<Gltf::Asset, ut::Error> Gltf::LoadAsset(const ut::JsonDoc& json)
{
	Asset asset;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find an asset node
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("asset"));
	if (!find_result)
	{
		return ut::MakeError(ut::error::fail, "JSon document has no gltf asset.");
	}

	// find a version field
	const ut::Tree<ut::text::Node>& json_asset = *find_result.Get();
	if (json_asset.CountChildren() == 0)
	{
		return ut::MakeError(ut::error::fail, "JSon document has an empty asset.");
	}

	find_result = ut::FindIf(json_asset.BeginLeaves(),
	                         json_asset.EndLeaves(),
		                     json_node_name_cmp("version"));
	if (!find_result)
	{
		return ut::MakeError(ut::error::fail, "Asset has no version field.");
	}

	// load a version value
	const ut::Optional<ut::String>& version = find_result.Get()->data.value;
	if (version)
	{
		asset.version = version.Get();
	}
	else
	{
		return ut::MakeError(ut::error::fail, "Asset version has no value.");
	}

	// minVersion
	find_result = ut::FindIf(json_asset.BeginLeaves(),
	                         json_asset.EndLeaves(),
		                     json_node_name_cmp("minVersion"));
	if (find_result)
	{
		asset.min_version = find_result.Get()->data.value;
	}

	// generator
	find_result = ut::FindIf(json_asset.BeginLeaves(),
	                         json_asset.EndLeaves(),
		                     json_node_name_cmp("generator"));
	if (find_result)
	{
		asset.generator = find_result.Get()->data.value;
	}

	// check version
	const bool version_support = asset.version == skSupportedVersion ||
		(asset.min_version && asset.min_version.Get() == skSupportedVersion);
	if (!version_support)
	{
		ut::String error_desc("Gltf version ");
		error_desc += asset.version +
			" is not supported. The only supported version is " +
			skSupportedVersion + ".";
		return ut::MakeError(ut::error::not_supported, ut::Move(error_desc));
	}

	// success
	return asset;
}

// Loads an array of GLTF scenes from the given JSON file.
ut::Result<ut::Array<Gltf::Scene>, ut::Error> Gltf::LoadScenes(const ut::JsonDoc& json)
{
	ut::Array<Gltf::Scene> scenes;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a scene array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("scenes"));
	if (!find_result)
	{
		return scenes;
	}

	// initialize scenes
	const ut::Tree<ut::text::Node>& json_scenes = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_scene : json_scenes)
	{
		Scene new_scene;

		// scene name
		find_result = ut::FindIf(json_scene.BeginLeaves(),
		                         json_scene.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& name_field = *find_result.Get();
			new_scene.name = name_field.data.value;
		}

		// scene nodes
		find_result = ut::FindIf(json_scene.BeginLeaves(),
		                         json_scene.EndLeaves(),
		                         json_node_name_cmp("nodes"));
		if (find_result)
		{
			ut::Array<ut::uint32> scene_nodes;

			const ut::Tree<ut::text::Node>& json_nodes = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& node : json_nodes)
			{
				if (node.data.value)
				{
					scene_nodes.Add(ut::Scan<ut::uint32>(node.data.value.Get()));
				}
			}

			new_scene.nodes = ut::Move(scene_nodes);
		}

		// add a new scene
		scenes.Add(ut::Move(new_scene));
	}

	// success
	return scenes;
}

// Loads the index of a GLTF scene to display from the given JSON file.
ut::Optional<ut::uint32> Gltf::LoadSceneId(const ut::JsonDoc& json)
{
	auto scene_id_cmd = [](const auto& node) { return node.data.name == "scene"; };
	auto scene_id_find_result = ut::FindIf(json.nodes.Begin(),
	                                       json.nodes.End(),
	                                       scene_id_cmd);
	if (scene_id_find_result && scene_id_find_result.Get()->data.value)
	{
		return ut::Scan<ut::uint32>(scene_id_find_result.Get()->data.value.Get());
	}

	return ut::Optional<ut::uint32>();
}

// Loads an array of GLTF nodes from the given JSON file.
ut::Result<ut::Array<Gltf::Node>, ut::Error> Gltf::LoadNodes(const ut::JsonDoc& json)
{
	ut::Array<Gltf::Node> nodes;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a node array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("nodes"));
	if (!find_result)
	{
		return nodes;
	}

	// initialize nodes
	const ut::Tree<ut::text::Node>& json_nodes = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_node : json_nodes)
	{
		Node new_node;

		// node name
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& name_field = *find_result.Get();
			new_node.name = name_field.data.value;
		}

		// clild nodes
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("children"));
		if (find_result)
		{
			ut::Array<ut::uint32> child_nodes;

			const ut::Tree<ut::text::Node>& json_child_nodes = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& child_node : json_child_nodes)
			{
				if (child_node.data.value)
				{
					child_nodes.Add(ut::Scan<ut::uint32>(child_node.data.value.Get()));
				}
			}

			new_node.children = ut::Move(child_nodes);
		}

		// matrix
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("matrix"));
		if (find_result)
		{
			ut::Array<float> matrix_data;

			const ut::Tree<ut::text::Node>& matrix_field = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& matrix_element : matrix_field)
			{
				if (matrix_element.data.value)
				{
					matrix_data.Add(ut::Scan<float>(matrix_element.data.value.Get()));
				}
			}

			if (matrix_data.Count() != 16)
			{
				return ut::MakeError(ut::error::fail, "Invalid matrix size.");
			}

			new_node.matrix = ut::Matrix<4, 4, float>();
			ut::memory::Copy(new_node.matrix->GetData(),
			                 matrix_data.GetAddress(),
			                 matrix_data.GetSize());
		}

		// rotation
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("rotation"));
		if (find_result)
		{
			ut::Array<float> quaternion_data;

			const ut::Tree<ut::text::Node>& rotation_field = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& rotation_element : rotation_field)
			{
				if (rotation_element.data.value)
				{
					quaternion_data.Add(ut::Scan<float>(rotation_element.data.value.Get()));
				}
			}

			if (quaternion_data.Count() != 4)
			{
				return ut::MakeError(ut::error::fail, "Invalid rotation quaternion size.");
			}

			new_node.rotation = ut::Quaternion<float>(quaternion_data[3],
			                                          quaternion_data[0],
			                                          quaternion_data[1],
			                                          quaternion_data[2]);
		}

		// translation
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("translation"));
		if (find_result)
		{
			ut::Array<float> translation_data;

			const ut::Tree<ut::text::Node>& translation_field = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& translation_element : translation_field)
			{
				if (translation_element.data.value)
				{
					translation_data.Add(ut::Scan<float>(translation_element.data.value.Get()));
				}
			}

			if (translation_data.Count() != 3)
			{
				return ut::MakeError(ut::error::fail, "Invalid translation vector size.");
			}

			new_node.translation = ut::Vector<3, float>(translation_data[0],
			                                            translation_data[1],
			                                            translation_data[2]);
		}

		// scale
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("scale"));
		if (find_result)
		{
			ut::Array<float> scale_data;

			const ut::Tree<ut::text::Node>& scale_field = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& scale_element : scale_field)
			{
				if (scale_element.data.value)
				{
					scale_data.Add(ut::Scan<float>(scale_element.data.value.Get()));
				}
			}

			if (scale_data.Count() != 3)
			{
				return ut::MakeError(ut::error::fail, "Invalid scale vector size.");
			}

			new_node.scale = ut::Vector<3, float>(scale_data[0],
			                                      scale_data[1],
			                                      scale_data[2]);
		}

		// weights
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("weights"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_weight_nodes = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& weight_node : json_weight_nodes)
			{
				if (weight_node.data.value)
				{
					new_node.weights.Add(ut::Scan<float>(weight_node.data.value.Get()));
				}
			}
		}

		// camera
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("camera"));
		if (find_result && find_result.Get()->data.value)
		{
			new_node.camera = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// skin
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("skin"));
		if (find_result && find_result.Get()->data.value)
		{
			new_node.skin = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// mesh
		find_result = ut::FindIf(json_node.BeginLeaves(),
		                         json_node.EndLeaves(),
		                         json_node_name_cmp("mesh"));
		if (find_result && find_result.Get()->data.value)
		{
			new_node.mesh = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// add a new node
		nodes.Add(ut::Move(new_node));
	}

	// success
	return nodes;
}

// Loads an array of GLTF meshes from the given JSON file.
ut::Result<ut::Array<Gltf::Mesh>, ut::Error> Gltf::LoadMeshes(const ut::JsonDoc& json_node)
{
	ut::Array<Gltf::Mesh> meshes;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a mesh array
	auto find_result = ut::FindIf(json_node.nodes.Begin(),
	                              json_node.nodes.End(),
	                              json_node_name_cmp("meshes"));
	if (!find_result)
	{
		return meshes;
	}

	// initialize meshes
	const ut::Tree<ut::text::Node>& json_meshes = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_mesh : json_meshes)
	{
		Mesh new_mesh;

		// mesh name
		find_result = ut::FindIf(json_mesh.BeginLeaves(),
		                         json_mesh.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& name_field = *find_result.Get();
			new_mesh.name = name_field.data.value;
		}

		// mesh primitives
		find_result = ut::FindIf(json_mesh.BeginLeaves(),
		                         json_mesh.EndLeaves(),
		                         json_node_name_cmp("primitives"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_primitives = *find_result.Get();
			ut::Result<ut::Array<Mesh::Primitive>,
			           ut::Error> primitives = LoadMeshPrimitives(json_primitives);
			if (!primitives)
			{
				return ut::MakeError(primitives.MoveAlt());
			}

			new_mesh.primitives = primitives.Move();
		}

		// weights
		find_result = ut::FindIf(json_mesh.BeginLeaves(),
		                         json_mesh.EndLeaves(),
		                         json_node_name_cmp("weights"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_weight_nodes = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& weight_node : json_weight_nodes)
			{
				if (weight_node.data.value)
				{
					new_mesh.weights.Add(ut::Scan<float>(weight_node.data.value.Get()));
				}
			}
		}

		// add a new mesh
		meshes.Add(ut::Move(new_mesh));
	}

	// success
	return meshes;
}

// Loads an array of GLTF mesh primitives from the given JSON file.
ut::Result<ut::Array<Gltf::Mesh::Primitive>,
           ut::Error> Gltf::LoadMeshPrimitives(const ut::Tree<ut::text::Node>& json_node)
{
	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	ut::Array<Gltf::Mesh::Primitive> primitives;

	for (const ut::Tree<ut::text::Node>& json_primitive : json_node)
	{
		Gltf::Mesh::Primitive new_primitive;

		// find attribute array
		auto find_result = ut::FindIf(json_primitive.BeginLeaves(),
		                              json_primitive.EndLeaves(),
		                              json_node_name_cmp("attributes"));
		if (!find_result)
		{
			return ut::MakeError(ut::error::fail, "Mesh primitive has no attributes.");
		}

		// initialize attributes
		const ut::Tree<ut::text::Node>& attribute_nodes = *find_result.Get();
		ut::Result<Gltf::Mesh::Primitive::Attributes, ut::Error> attributes_result =
			LoadPrimitiveAttributes(attribute_nodes);
		if (!attributes_result)
		{
			return ut::MakeError(attributes_result.MoveAlt());
		}
		new_primitive.attributes = attributes_result.Move();

		// morph targets
		find_result = ut::FindIf(json_primitive.BeginLeaves(),
		                         json_primitive.EndLeaves(),
		                         json_node_name_cmp("targets"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& morph_targets = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& morph_target : morph_targets)
			{
				attributes_result = LoadPrimitiveAttributes(morph_target);
				if (!attributes_result)
				{
					return ut::MakeError(attributes_result.MoveAlt());
				}
				new_primitive.morph_targets.Add(attributes_result.Move());
			}
		}

		// indices
		find_result = ut::FindIf(json_primitive.BeginLeaves(),
		                         json_primitive.EndLeaves(),
		                         json_node_name_cmp("indices"));
		if (find_result && find_result.Get()->data.value)
		{
			new_primitive.indices = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// material
		find_result = ut::FindIf(json_primitive.BeginLeaves(),
		                         json_primitive.EndLeaves(),
		                         json_node_name_cmp("material"));
		if (find_result && find_result.Get()->data.value)
		{
			new_primitive.material = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// mode
		find_result = ut::FindIf(json_primitive.BeginLeaves(),
		                         json_primitive.EndLeaves(),
		                         json_node_name_cmp("mode"));
		if (find_result && find_result.Get()->data.value)
		{
			const ut::uint32 mode_value = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
			if (mode_value > static_cast<ut::uint32>(Mesh::Primitive::Mode::max_value))
			{
				return ut::MakeError(ut::error::fail, "Mesh Primitive Mode has invalid value.");
			}

			new_primitive.mode = static_cast<Mesh::Primitive::Mode>(mode_value);
		}

		// add a new primitive
		primitives.Add(ut::Move(new_primitive));
	}

	return primitives;
}

// Loads an array of GLTF mesh primitive attributes from the given JSON file.
ut::Result<Gltf::Mesh::Primitive::Attributes,
           ut::Error> Gltf::LoadPrimitiveAttributes(const ut::Tree<ut::text::Node>& node)
{
	Gltf::Mesh::Primitive::Attributes attributes;

	for (const ut::Tree<ut::text::Node>& attribute_node : node)
	{
		if (attribute_node.data.name.Length() == 0)
		{
			return ut::MakeError(ut::error::fail, "Mesh primitive attribute has no name.");
		}

		if (!attribute_node.data.value)
		{
			return ut::MakeError(ut::error::fail, "Mesh primitive attribute has empty value.");
		}

		ut::String attribute_name(attribute_node.data.name);
		const ut::uint32 attribute_id =
			ut::Scan<ut::uint32>(attribute_node.data.value.Get());
		Gltf::Mesh::Primitive::Attribute attribute(ut::Move(attribute_name),
			attribute_id);
		attributes.Add(ut::Move(attribute));
	}

	return attributes;
}

// Loads an array of GLTF accessors from the given JSON file.
ut::Result<ut::Array<Gltf::Accessor>,
           ut::Error> Gltf::LoadAccessors(const ut::JsonDoc& json)
{
	ut::Array<Accessor> accessors;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find an accessor array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("accessors"));
	if (!find_result)
	{
		return accessors;
	}

	// initialize accessors
	const ut::Tree<ut::text::Node>& json_accessors = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_accessor : json_accessors)
	{
		Accessor new_accessor;

		// componentType
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("componentType"));
		if (!find_result || !find_result.Get()->data.value)
		{
			return ut::MakeError(ut::error::fail, "Accessor has no \"componentType\" field.");
		}
		const ut::uint32 component_type_value = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		new_accessor.component_type = static_cast<Accessor::ComponentType>(component_type_value);

		// count
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("count"));
		if (!find_result || !find_result.Get()->data.value)
		{
			return ut::MakeError(ut::error::fail, "Accessor has no \"count\" field.");
		}
		new_accessor.count = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());

		// type
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("type"));
		if (!find_result || !find_result.Get()->data.value)
		{
			return ut::MakeError(ut::error::fail, "Accessor has no \"type\" field.");
		}
		new_accessor.type = find_result.Get()->data.value.Get();

		// name
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& name_field = *find_result.Get();
			new_accessor.name = name_field.data.value;
		}

		// bufferView
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("bufferView"));
		if (find_result && find_result.Get()->data.value)
		{
			new_accessor.buffer_view = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// byteOffset
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("byteOffset"));
		if (find_result && find_result.Get()->data.value)
		{
			new_accessor.byte_offset = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// normalized
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("normalized"));
		if (find_result && find_result.Get()->data.value)
		{
			new_accessor.normalized = ut::Scan<bool>(find_result.Get()->data.value.Get());
		}

		// min
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("min"));
		if (find_result)
		{
			new_accessor.min_component_value = ut::Array<float>();

			const ut::Tree<ut::text::Node>& json_min_elements = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& min_element : json_min_elements)
			{
				if (min_element.data.value)
				{
					new_accessor.min_component_value->Add(ut::Scan<float>(min_element.data.value.Get()));
				}
				else
				{
					return ut::MakeError(ut::error::fail, "Invalid Accessor's \"min\" value.");
				}
			}
		}

		// max
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("max"));
		if (find_result)
		{
			new_accessor.max_component_value = ut::Array<float>();

			const ut::Tree<ut::text::Node>& json_max_elements = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& max_element : json_max_elements)
			{
				if (max_element.data.value)
				{
					new_accessor.max_component_value->Add(ut::Scan<float>(max_element.data.value.Get()));
				}
				else
				{
					return ut::MakeError(ut::error::fail, "Invalid Accessor's \"max\" value.");
				}
			}
		}

		// sparse
		find_result = ut::FindIf(json_accessor.BeginLeaves(),
		                         json_accessor.EndLeaves(),
		                         json_node_name_cmp("sparse"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_sparse = *find_result.Get();
			ut::Result<Accessor::Sparse, ut::Error> sparse = LoadAccessorSparse(json_sparse);
			if (!sparse)
			{
				return ut::MakeError(sparse.MoveAlt());
			}

			new_accessor.sparse = sparse.Move();
		}

		// add the accessor to the array
		accessors.Add(ut::Move(new_accessor));
	}

	return accessors;
}

// Loads a GLTF accessor sparse object from the given JSON file.
ut::Result<Gltf::Accessor::Sparse,
           ut::Error> Gltf::LoadAccessorSparse(const ut::Tree<ut::text::Node>& json_node)
{
	Accessor::Sparse sparse;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// count
	auto find_result = ut::FindIf(json_node.BeginLeaves(),
	                              json_node.EndLeaves(),
	                              json_node_name_cmp("count"));
	if (find_result && find_result.Get()->data.value)
	{
		sparse.count = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
	}
	else
	{
		return ut::MakeError(ut::error::fail, "Accessor Sparse object has no \"count\" field.");
	}

	// indices
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("indices"));
	if (find_result)
	{
		const ut::Tree<ut::text::Node>& json_indices = *find_result.Get();

		// bufferView
		find_result = ut::FindIf(json_indices.BeginLeaves(),
		                         json_indices.EndLeaves(),
		                         json_node_name_cmp("bufferView"));
		if (find_result && find_result.Get()->data.value)
		{
			sparse.indices.buffer_view = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}
		else
		{
			return ut::MakeError(ut::error::fail, "Accessor Sparse Indices object has no \"bufferView\" field.");
		}

		// componentType
		find_result = ut::FindIf(json_indices.BeginLeaves(),
		                         json_indices.EndLeaves(),
		                         json_node_name_cmp("componentType"));
		if (find_result && find_result.Get()->data.value)
		{
			sparse.indices.component_type = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}
		else
		{
			return ut::MakeError(ut::error::fail, "Accessor Sparse Indices object has no \"componentType\" field.");
		}

		// byteOffset
		find_result = ut::FindIf(json_indices.BeginLeaves(),
		                         json_indices.EndLeaves(),
		                         json_node_name_cmp("byteOffset"));
		if (find_result && find_result.Get()->data.value)
		{
			sparse.indices.byte_offset = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}
	}
	else
	{
		return ut::MakeError(ut::error::fail, "Accessor Sparse object has no \"indices\" field.");
	}

	// values
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("values"));
	if (find_result)
	{
		const ut::Tree<ut::text::Node>& json_values = *find_result.Get();

		// bufferView
		find_result = ut::FindIf(json_values.BeginLeaves(),
		                         json_values.EndLeaves(),
		                         json_node_name_cmp("bufferView"));
		if (find_result && find_result.Get()->data.value)
		{
			sparse.values.buffer_view = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}
		else
		{
			return ut::MakeError(ut::error::fail, "Accessor Sparse Values object has no \"bufferView\" field.");
		}

		// byteOffset
		find_result = ut::FindIf(json_values.BeginLeaves(),
		                         json_values.EndLeaves(),
		                         json_node_name_cmp("byteOffset"));
		if (find_result && find_result.Get()->data.value)
		{
			sparse.values.byte_offset = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}
	}
	else
	{
		return ut::MakeError(ut::error::fail, "Accessor Sparse object has no \"values\" field.");
	}

	return sparse;
}

// Loads an array of GLTF buffers from the given JSON file.
ut::Result<ut::Array<Gltf::Buffer>, ut::Error> Gltf::LoadBuffers(const ut::JsonDoc& json,
                                                                 const ut::String& root_dir)
{
	ut::Array<Buffer> buffers;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a buffer array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("buffers"));
	if (!find_result)
	{
		return buffers;
	}

	// initialize buffers
	const ut::Tree<ut::text::Node>& json_buffers = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_buffer : json_buffers)
	{
		Buffer new_buffer;

		// byte length
		find_result = ut::FindIf(json_buffer.BeginLeaves(),
		                         json_buffer.EndLeaves(),
		                         json_node_name_cmp("byteLength"));
		if (!find_result || !find_result.Get()->data.value)
		{
			return ut::MakeError(ut::error::fail, "Buffer has no \"byteLength\" field.");
		}
		new_buffer.byte_length = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		new_buffer.data.Resize(new_buffer.byte_length);

		// uri
		find_result = ut::FindIf(json_buffer.BeginLeaves(),
		                         json_buffer.EndLeaves(),
		                         json_node_name_cmp("uri"));
		if (find_result)
		{
			new_buffer.uri = find_result.Get()->data.value;
			if (new_buffer.uri)
			{
				ut::Result<ut::Array<ut::byte>,
				           ut::Error> data = LoadUri(new_buffer.uri.Get(),
				                                     root_dir);
				if (!data)
				{
					return ut::MakeError(data.MoveAlt());
				}

				new_buffer.data = data.Move();
			}
			else
			{
				return ut::MakeError(ut::error::empty, "Buffer URI is empty.");
			}
		}
		else
		{
			ut::memory::Set(new_buffer.data.GetAddress(), 0, new_buffer.byte_length);
		}

		// name
		find_result = ut::FindIf(json_buffer.BeginLeaves(),
		                         json_buffer.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			new_buffer.name = find_result.Get()->data.value;
		}

		// add this buffer to the array
		buffers.Add(ut::Move(new_buffer));
	}

	return buffers;
}

// Loads binary data from the given GLTF URI link.
ut::Result<ut::Array<ut::byte>,
           ut::Error> Gltf::LoadUri(const ut::String& uri,
                                    const ut::String& root_dir)
{
	// check if data is embedded in uri
	const char* data_label = "data:";
	const char* embedded_addr = ut::StrStr(uri.GetAddress(), data_label);
	if (embedded_addr == uri.GetAddress())
	{
		return ut::MakeError(ut::error::not_supported,
		                     "Embedded URI is not supported.");
	}

	// open file
	const ut::String file_path = root_dir + uri;
	ut::File file;
	ut::Optional<ut::Error> file_error = file.Open(file_path,
	                                               ut::File::Access::read);
	if (file_error)
	{
		return ut::MakeError(file_error.Move());
	}

	// get file size
	ut::Result<size_t, ut::Error> get_size_result = file.GetSize();
	if (!get_size_result)
	{
		return ut::MakeError(get_size_result.GetAlt());
	}
	const size_t file_size = get_size_result.Get();

	// read file content
	ut::Array<ut::byte> data(file_size);
	file_error = file.Read(data.GetAddress(), 1, file_size);
	if (file_error)
	{
		return ut::MakeError(file_error.Move());
	}

	// close file
	file.Close();

	// success
	return data;
}

// Loads an array of GLTF buffer views from the given JSON file.
ut::Result<ut::Array<Gltf::BufferView>,
           ut::Error> Gltf::LoadBufferViews(const ut::JsonDoc& json)
{
	ut::Array<BufferView> buffer_views;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a bufferViews array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("bufferViews"));
	if (!find_result)
	{
		return buffer_views;
	}

	// initialize buffer views
	const ut::Tree<ut::text::Node>& json_bufviews = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_bufview : json_bufviews)
	{
		BufferView new_bufview;

		// buffer id
		find_result = ut::FindIf(json_bufview.BeginLeaves(),
		                         json_bufview.EndLeaves(),
		                         json_node_name_cmp("buffer"));
		if (!find_result || !find_result.Get()->data.value)
		{
			return ut::MakeError(ut::error::fail,
			                     "Buffer View has no \"buffer\" field.");
		}
		new_bufview.buffer_id =
			ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());

		// byte length
		find_result = ut::FindIf(json_bufview.BeginLeaves(),
		                         json_bufview.EndLeaves(),
		                         json_node_name_cmp("byteLength"));
		if (!find_result || !find_result.Get()->data.value)
		{
			return ut::MakeError(ut::error::fail,
			                     "Buffer View has no \"byteLength\" field.");
		}
		new_bufview.byte_length =
			ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());

		// byte offset
		find_result = ut::FindIf(json_bufview.BeginLeaves(),
		                         json_bufview.EndLeaves(),
		                         json_node_name_cmp("byteOffset"));
		if (find_result && find_result.Get()->data.value)
		{
			new_bufview.byte_offset =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// byte stride
		find_result = ut::FindIf(json_bufview.BeginLeaves(),
		                         json_bufview.EndLeaves(),
		                         json_node_name_cmp("byteStride"));
		if (find_result && find_result.Get()->data.value)
		{
			new_bufview.byte_stride =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// target
		find_result = ut::FindIf(json_bufview.BeginLeaves(),
		                         json_bufview.EndLeaves(),
		                         json_node_name_cmp("target"));
		if (find_result && find_result.Get()->data.value)
		{
			new_bufview.target =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// name
		find_result = ut::FindIf(json_bufview.BeginLeaves(),
		                         json_bufview.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			new_bufview.name = find_result.Get()->data.value;
		}

		// add this buffer view to the array
		buffer_views.Add(ut::Move(new_bufview));
	}

	return buffer_views;
}

// Loads an array of GLTF images from the given JSON file.
ut::Result<ut::Array<Gltf::Image>,
           ut::Error> Gltf::LoadImages(const ut::JsonDoc& json)
{
	ut::Array<Image> images;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find an image array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("images"));
	if (!find_result)
	{
		return images;
	}

	// initialize images
	const ut::Tree<ut::text::Node>& json_images = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_image : json_images)
	{
		Image new_image;

		// uri
		find_result = ut::FindIf(json_image.BeginLeaves(),
		                         json_image.EndLeaves(),
		                         json_node_name_cmp("uri"));
		if (find_result)
		{
			new_image.uri = find_result.Get()->data.value;
		}

		// buffer view
		find_result = ut::FindIf(json_image.BeginLeaves(),
		                         json_image.EndLeaves(),
		                         json_node_name_cmp("bufferView"));
		if (find_result && find_result.Get()->data.value)
		{
			new_image.buffer_view =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// mime type
		find_result = ut::FindIf(json_image.BeginLeaves(),
		                         json_image.EndLeaves(),
		                         json_node_name_cmp("mimeType"));
		if (find_result)
		{
			new_image.mime_type = find_result.Get()->data.value;
		}

		// name
		find_result = ut::FindIf(json_image.BeginLeaves(),
		                         json_image.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			new_image.name = find_result.Get()->data.value;
		}

		// add this image view to the array
		images.Add(ut::Move(new_image));
	}

	return images;
}

// Loads an array of GLTF samplers from the given JSON file.
ut::Result<ut::Array<Gltf::Sampler>, ut::Error> Gltf::LoadSamplers(const ut::JsonDoc& json)
{
	ut::Array<Sampler> samplers;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a sampler array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("samplers"));
	if (!find_result)
	{
		return samplers;
	}

	// initialize samplers
	const ut::Tree<ut::text::Node>& json_samplers = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_sampler : json_samplers)
	{
		Sampler new_sampler;

		// name
		find_result = ut::FindIf(json_sampler.BeginLeaves(),
		                         json_sampler.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			new_sampler.name = find_result.Get()->data.value;
		}

		// magnification filter
		find_result = ut::FindIf(json_sampler.BeginLeaves(),
		                         json_sampler.EndLeaves(),
		                         json_node_name_cmp("magFilter"));
		if (find_result && find_result.Get()->data.value)
		{
			new_sampler.mag_filter =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// minification filter
		find_result = ut::FindIf(json_sampler.BeginLeaves(),
		                         json_sampler.EndLeaves(),
		                         json_node_name_cmp("minFilter"));
		if (find_result && find_result.Get()->data.value)
		{
			new_sampler.min_filter =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// S(U) wrapping mode
		find_result = ut::FindIf(json_sampler.BeginLeaves(),
		                         json_sampler.EndLeaves(),
		                         json_node_name_cmp("wrapS"));
		if (find_result && find_result.Get()->data.value)
		{
			new_sampler.wrap_su =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// T(V) wrapping mode
		find_result = ut::FindIf(json_sampler.BeginLeaves(),
		                         json_sampler.EndLeaves(),
		                         json_node_name_cmp("wrapT"));
		if (find_result && find_result.Get()->data.value)
		{
			new_sampler.wrap_tv =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// add this sampler to the array
		samplers.Add(ut::Move(new_sampler));
	}

	return samplers;
}

// Loads an array of GLTF textures from the given JSON file.
ut::Result<ut::Array<Gltf::Texture>,
           ut::Error> Gltf::LoadTextures(const ut::JsonDoc& json)
{
	ut::Array<Texture> textures;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a texture array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("textures"));
	if (!find_result)
	{
		return textures;
	}

	// initialize samplers
	const ut::Tree<ut::text::Node>& json_textures = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_texture : json_textures)
	{
		Texture new_texture;

		// name
		find_result = ut::FindIf(json_texture.BeginLeaves(),
		                         json_texture.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			new_texture.name = find_result.Get()->data.value;
		}

		// sampler
		find_result = ut::FindIf(json_texture.BeginLeaves(),
		                         json_texture.EndLeaves(),
		                         json_node_name_cmp("sampler"));
		if (find_result && find_result.Get()->data.value)
		{
			new_texture.sampler =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// source
		find_result = ut::FindIf(json_texture.BeginLeaves(),
		                         json_texture.EndLeaves(),
		                         json_node_name_cmp("source"));
		if (find_result && find_result.Get()->data.value)
		{
			new_texture.source =
				ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
		}

		// add this texture to the array
		textures.Add(ut::Move(new_texture));
	}

	return textures;
}

// Loads an array of GLTF materials from the given JSON file.
ut::Result<ut::Array<Gltf::Material>,
           ut::Error> Gltf::LoadMaterials(const ut::JsonDoc& json)
{
	ut::Array<Material> materials;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// find a material array
	auto find_result = ut::FindIf(json.nodes.Begin(),
	                              json.nodes.End(),
	                              json_node_name_cmp("materials"));
	if (!find_result)
	{
		return materials;
	}

	// initialize materials
	const ut::Tree<ut::text::Node>& json_materials = *find_result.Get();
	for (const ut::Tree<ut::text::Node>& json_material : json_materials)
	{
		Material new_material;

		// name
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("name"));
		if (find_result)
		{
			new_material.name = find_result.Get()->data.value;
		}

		// alpha mode
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("alphaMode"));
		if (find_result)
		{
			new_material.alpha_mode = find_result.Get()->data.value;
		}

		// alpha cut-off
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("alphaCutoff"));
		if (find_result && find_result.Get()->data.value)
		{
			new_material.alpha_cutoff =
				ut::Scan<float>(find_result.Get()->data.value.Get());
		}

		// double-sided
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("doubleSided"));
		if (find_result && find_result.Get()->data.value)
		{
			new_material.double_sided =
				ut::Scan<bool>(find_result.Get()->data.value.Get());
		}

		// emissive factor
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("emissiveFactor"));
		if (find_result)
		{
			ut::Array<float> emissive_data;

			const ut::Tree<ut::text::Node>& emissive_field = *find_result.Get();
			for (const ut::Tree<ut::text::Node>& emissive_channel : emissive_field)
			{
				if (emissive_channel.data.value)
				{
					emissive_data.Add(ut::Scan<float>(emissive_channel.data.value.Get()));
				}
			}

			if (emissive_data.Count() != 3)
			{
				return ut::MakeError(ut::error::fail, "Invalid emissive factor vector size.");
			}

			new_material.emissive_factor = ut::Vector<3, float>(emissive_data[0],
			                                                    emissive_data[1],
			                                                    emissive_data[2]);
		}

		// emissive texture
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("emissiveTexture"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_emissive_texture = *find_result.Get();
			ut::Result<TextureInfo, ut::Error> emissive_texture = LoadTextureInfo(json_emissive_texture);
			if (!emissive_texture)
			{
				return ut::MakeError(emissive_texture.MoveAlt());
			}

			new_material.emissive_texture = emissive_texture.Move();
		}

		// pbr metallic roughness
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("pbrMetallicRoughness"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_pbr_mr = *find_result.Get();
			ut::Result<Material::PbrMetallicRoughness, ut::Error> pbr_mr = LoadPbrMetallicRoughness(json_pbr_mr);
			if (!pbr_mr)
			{
				return ut::MakeError(pbr_mr.MoveAlt());
			}

			new_material.pbr_metallic_roughness = pbr_mr.Move();
		}

		// normal texture info
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("normalTexture"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_normal_texture = *find_result.Get();
			ut::Result<Material::NormalTextureInfo, ut::Error> normal_texture = LoadNormalTextureInfo(json_normal_texture);
			if (!normal_texture)
			{
				return ut::MakeError(normal_texture.MoveAlt());
			}

			new_material.normal_texture = normal_texture.Move();
		}

		// occlusion texture info
		find_result = ut::FindIf(json_material.BeginLeaves(),
		                         json_material.EndLeaves(),
		                         json_node_name_cmp("occlusionTexture"));
		if (find_result)
		{
			const ut::Tree<ut::text::Node>& json_occlusion_texture = *find_result.Get();
			ut::Result<Material::OcclusionTextureInfo, ut::Error> occlusion_texture = LoadOcclusionTextureInfo(json_occlusion_texture);
			if (!occlusion_texture)
			{
				return ut::MakeError(occlusion_texture.MoveAlt());
			}

			new_material.occlusion_texture = occlusion_texture.Move();
		}

		// add this material view to the array
		materials.Add(ut::Move(new_material));
	}

	return materials;
}

// Loads the desired GLTF texture information from the given JSON file.
ut::Result<Gltf::TextureInfo,
           ut::Error> Gltf::LoadTextureInfo(const ut::Tree<ut::text::Node>& json_node)
{
	TextureInfo texture_info;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// index
	auto find_result = ut::FindIf(json_node.BeginLeaves(),
	                              json_node.EndLeaves(),
	                              json_node_name_cmp("index"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.index = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
	}
	else
	{
		return ut::MakeError(ut::error::fail,
		                     "Texture Info object has no \"index\" field.");
	}

	// texture coordinates
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("texCoord"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.texcoord =
			ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
	}

	return texture_info;
}

// Loads the desired GLTF metallic-roughness object from the given JSON file.
ut::Result<Gltf::Material::PbrMetallicRoughness,
           ut::Error> Gltf::LoadPbrMetallicRoughness(const ut::Tree<ut::text::Node>& json_node)
{
	Material::PbrMetallicRoughness pbr_mr;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// base color factor
	auto find_result = ut::FindIf(json_node.BeginLeaves(),
	                              json_node.EndLeaves(),
	                              json_node_name_cmp("baseColorFactor"));
	if (find_result)
	{
		ut::Array<float> base_color_data;

		const ut::Tree<ut::text::Node>& json_base_color = *find_result.Get();
		for (const ut::Tree<ut::text::Node>& base_color_channel : json_base_color)
		{
			if (base_color_channel.data.value)
			{
				base_color_data.Add(ut::Scan<float>(base_color_channel.data.value.Get()));
			}
		}

		if (base_color_data.Count() != 4)
		{
			return ut::MakeError(ut::error::fail, "Invalid pbr base color factor vector size.");
		}

		pbr_mr.base_color_factor = ut::Vector<4, float>(base_color_data[0],
		                                                base_color_data[1],
		                                                base_color_data[2],
		                                                base_color_data[3]);
	}

	// metallic factor
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("metallicFactor"));
	if (find_result && find_result.Get()->data.value)
	{
		pbr_mr.metallic_factor = ut::Scan<float>(find_result.Get()->data.value.Get());
	}

	// roughness factor
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("roughnessFactor"));
	if (find_result && find_result.Get()->data.value)
	{
		pbr_mr.roughness_factor =
			ut::Scan<float>(find_result.Get()->data.value.Get());
	}

	// base color texture
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("baseColorTexture"));
	if (find_result)
	{
		const ut::Tree<ut::text::Node>& json_base_color_texture = *find_result.Get();
		ut::Result<TextureInfo, ut::Error> base_color_texture =
			LoadTextureInfo(json_base_color_texture);
		if (!base_color_texture)
		{
			return ut::MakeError(base_color_texture.MoveAlt());
		}

		pbr_mr.base_color_texture = base_color_texture.Move();
	}

	// metallic roughness texture
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("metallicRoughnessTexture"));
	if (find_result)
	{
		const ut::Tree<ut::text::Node>& json_metallic_roughness_texture =
			*find_result.Get();
		ut::Result<TextureInfo, ut::Error> metallic_roughness_texture =
			LoadTextureInfo(json_metallic_roughness_texture);
		if (!metallic_roughness_texture)
		{
			return ut::MakeError(metallic_roughness_texture.MoveAlt());
		}

		pbr_mr.metallic_roughness_texture = metallic_roughness_texture.Move();
	}

	return pbr_mr;
}

// Loads the desired GLTF normal map information from the given JSON file.
ut::Result<Gltf::Material::NormalTextureInfo,
           ut::Error> Gltf::LoadNormalTextureInfo(const ut::Tree<ut::text::Node>& json_node)
{
	Material::NormalTextureInfo texture_info;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// index
	auto find_result = ut::FindIf(json_node.BeginLeaves(),
	                              json_node.EndLeaves(),
	                              json_node_name_cmp("index"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.index = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
	}
	else
	{
		return ut::MakeError(ut::error::fail,
		                     "Normal Texture Info object has no \"index\" field.");
	}

	// texture coordinates
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("texCoord"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.texcoord =
			ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
	}

	// scale
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("scale"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.scale =
			ut::Scan<float>(find_result.Get()->data.value.Get());
	}

	return texture_info;
}

// Loads the desired GLTF occlusion map information from the given JSON file.
ut::Result<Gltf::Material::OcclusionTextureInfo,
           ut::Error> Gltf::LoadOcclusionTextureInfo(const ut::Tree<ut::text::Node>& json_node)
{
	Material::OcclusionTextureInfo texture_info;

	auto json_node_name_cmp = [](const ut::String& node_name)
	{
		return [=](const auto& node) { return node.data.name == node_name; };
	};

	// index
	auto find_result = ut::FindIf(json_node.BeginLeaves(),
	                              json_node.EndLeaves(),
	                              json_node_name_cmp("index"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.index = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
	}
	else
	{
		return ut::MakeError(ut::error::fail,
		                     "Occlusion Texture Info object has no \"index\" field.");
	}

	// texture coordinates
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("texCoord"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.texcoord = ut::Scan<ut::uint32>(find_result.Get()->data.value.Get());
	}

	// strength
	find_result = ut::FindIf(json_node.BeginLeaves(),
	                         json_node.EndLeaves(),
	                         json_node_name_cmp("strength"));
	if (find_result && find_result.Get()->data.value)
	{
		texture_info.strength = ut::Scan<float>(find_result.Get()->data.value.Get());
	}

	return texture_info;
}

// Constructor.
MeshLoader::MeshLoader(Device& device_ref,
                       ResourceManager& rc_mgr_ref) noexcept : device(device_ref)
                                                             , rc_mgr(rc_mgr_ref)
{}

// Loads a mesh from a file.
//    @param filename - path to the mesh file to be loaded.
//    @param info - const reference to the ImageLoader::Info object.
//    @return - new render::Image object or ut::Error if failed.
ut::Result<Mesh, ut::Error> MeshLoader::Load(const ut::String& filename)
{
	try
	{
		Gltf gltf(filename);
		return gltf.ExportMesh(device, rc_mgr);
	}
	catch (ut::Error error)
	{
		return ut::MakeError(ut::Move(error));
	}

	return ut::MakeError(ut::error::not_supported);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//