//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/ve_render_batcher.h"
#include "systems/render/engine/units/ve_render_directional_light.h"
#include "systems/render/engine/units/ve_render_point_light.h"
#include "systems/render/engine/units/ve_render_spot_light.h"
#include "templates/ut_grid.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Encapsulates forward shading techniques. Forward renderer draws objects that
// can't be rendered via deferred rendering (such as transparent mesh instances).
class ForwardShading
{
public:
	// Per-view gpu data.
	struct ViewData
	{
		ut::Array<Framebuffer> light_framebuffer;

		// secondary buffers to parallelize cpu work
		// note that cubemaps need a separate set of buffers for each face
		ut::Array<CmdBuffer> lightpass_cmd;
	};

	// Constructor.
	ForwardShading(Toolset& toolset, ut::uint32 ibl_mip_count);

	// Creates forward shading (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param light_buffer - reference to the light buffer.
	//    @param is_cube - 'true' to create as a cubemap.
	//    @return - a new ForwardShading::ViewData object or error if failed.
	ut::Result<ForwardShading::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                               Target& light_buffer,
	                                                               bool is_cube);

	// Renders transparent units directly to the light buffer.
	void DrawTransparentGeometry(Context& context,
	                             ForwardShading::ViewData& data,
	                             Buffer& view_uniform_buffer,
	                             const ut::Vector<3>& view_position,
	                             Batcher& batcher,
	                             Light::Sources& lights,
	                             ut::Optional<Image&> ibl_cubemap,
	                             Image::Cube::Face cubeface = Image::Cube::Face::positive_x);

private:
	// Light pass pipeline permutations.
	struct LightPass
	{
		struct MeshInstRendering
		{
			enum class AlphaTest
			{
				off,
				on,
				count
			};

			enum class AlphaMode
			{
				blend,
				add,
				count
			};

			enum class IblPreset
			{
				off,
				on,
				count
			};

			enum class CullMode
			{
				front,
				back,
				count
			};

			enum class StencilMode
			{
				none,
				highlighted,
				count
			};

			static constexpr ut::uint32 vertex_format_column = 0;
			static constexpr ut::uint32 ibl_column = 1;
			static constexpr ut::uint32 light_type_column = 2;
			static constexpr ut::uint32 alpha_test_column = 3;
			static constexpr ut::uint32 alpha_mode_column = 4;
			static constexpr ut::uint32 cull_mode_column = 5;
			static constexpr ut::uint32 stencil_mode_column = 6;
			static constexpr ut::uint32 polygon_mode_column = 7;

			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(IblPreset::count),
			                 static_cast<size_t>(Light::SourceType::count),
			                 static_cast<size_t>(AlphaTest::count),
			                 static_cast<size_t>(AlphaMode::count),
			                 static_cast<size_t>(CullMode::count),
			                 static_cast<size_t>(StencilMode::count),
			                 static_cast<size_t>(Mesh::PolygonMode::count)> PipelineGrid;

			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(IblPreset::count),
			                 static_cast<size_t>(Light::SourceType::count),
			                 static_cast<size_t>(AlphaTest::count)> ShaderGrid;

			// Descriptor set for the light pass shaders.
			struct Descriptors : public DescriptorSet
			{
				Descriptors() : DescriptorSet(view_ub, light_ub, transform_ub,
				                              material_ub, sampler, diffuse,
				                              normal, material)
				{}

				Descriptor view_ub = "g_ub_view";
				Descriptor light_ub = "g_ub_light";
				Descriptor transform_ub = "g_ub_transform";
				Descriptor material_ub = "g_ub_material";
				Descriptor sampler = "g_sampler";
				Descriptor diffuse = "g_tex2d_diffuse";
				Descriptor normal = "g_tex2d_normal";
				Descriptor material = "g_tex2d_material";
			};
		};
	};

	// IBL pass pipeline permutations.
	struct IblPass
	{
		struct MeshInstRendering
		{
			enum class AlphaTest
			{
				off,
				on,
				count
			};

			enum class AlphaMode
			{
				blend,
				add,
				count
			};

			enum class CullMode
			{
				front,
				back,
				count
			};

			enum class StencilMode
			{
				none,
				highlighted,
				count
			};

			static constexpr ut::uint32 vertex_format_column = 0;
			static constexpr ut::uint32 alpha_test_column = 1;
			static constexpr ut::uint32 alpha_mode_column = 2;
			static constexpr ut::uint32 cull_mode_column = 3;
			static constexpr ut::uint32 stencil_mode_column = 4;
			static constexpr ut::uint32 polygon_mode_column = 5;

			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count),
			                 static_cast<size_t>(AlphaMode::count),
			                 static_cast<size_t>(CullMode::count),
			                 static_cast<size_t>(StencilMode::count),
			                 static_cast<size_t>(Mesh::PolygonMode::count)> PipelineGrid;

			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count)> ShaderGrid;

			// Descriptor set for the ibl pass shaders.
			struct Descriptors : public DescriptorSet
			{
				Descriptors() : DescriptorSet(view_ub, transform_ub,
				                              material_ub, sampler,
				                              diffuse, normal,
				                              material, ibl_cubemap)
				{}

				Descriptor view_ub = "g_ub_view";
				Descriptor transform_ub = "g_ub_transform";
				Descriptor material_ub = "g_ub_material";
				Descriptor sampler = "g_sampler";
				Descriptor diffuse = "g_tex2d_diffuse";
				Descriptor normal = "g_tex2d_normal";
				Descriptor material = "g_tex2d_material";
				Descriptor ibl_cubemap = "g_ibl_cubemap";
			};
		};
	};

	// Renders specified range of mesh instances.
	void RenderTransparentMeshInstancesJob(Context& context,
	                                       ForwardShading::ViewData& data,
	                                       Light::Sources& lights,
	                                       Buffer& view_uniform_buffer,
	                                       Batcher& batcher,
	                                       ut::Optional<Image&> ibl_cubemap,
	                                       ut::uint32 thread_id,
	                                       ut::uint32 offset,
	                                       ut::uint32 count);

	// Applies direct lighting to the specified mesh instance.
	void RenderTransparentMeshInstanceLights(Context& context,
	                                         ForwardShading::ViewData& data,
	                                         Light::Sources& lights,
	                                         Buffer& view_uniform_buffer,
	                                         Batcher& batcher,
	                                         LightPass::MeshInstRendering::IblPreset ibl_preset,
	                                         LightPass::MeshInstRendering::CullMode cull_mode,
	                                         LightPass::MeshInstRendering::AlphaMode alpha_mode,
	                                         ut::uint32 drawcall_id,
	                                         ut::uint32 thread_id);

	// Applies direct lighting to the specified mesh instance.
	void RenderTransparentMeshInstanceIbl(Context& context,
	                                      ForwardShading::ViewData& data,
	                                      Image& ibl_cubemap,
	                                      Buffer& view_uniform_buffer,
	                                      Batcher& batcher,
	                                      IblPass::MeshInstRendering::CullMode cull_mode,
	                                      IblPass::MeshInstRendering::AlphaMode alpha_mode,
	                                      ut::uint32 drawcall_id,
	                                      ut::uint32 thread_id);

	// Renders provided mesh.
	static void DrawMesh(Context& context,
	                     Mesh& mesh,
	                     PipelineState& pipeline,
	                     DescriptorSet& desc_set,
	                     Buffer& instance_buffer,
	                     ut::uint32 index_count,
	                     ut::uint32 index_offset,
	                     ut::uint32 instance_offset);

	// Creates a shader for the lighting pass.
	ut::Array<BoundShader> CreateMeshInstLightPassShader();

	// Creates a shader for the ibl pass.
	ut::Array<BoundShader> CreateMeshInstIblShader(ut::uint32 ibl_mip_count);

	// Creates a render pass for the shading techniques.
	RenderPass CreateLightPass();

	// Creates a pipeline state to apply lighting.
	ut::Result<PipelineState, ut::Error> CreateMeshInstLightPassPipeline(Mesh::VertexFormat vertex_format,
	                                                                     Mesh::PolygonMode polygon_mode,
	                                                                     LightPass::MeshInstRendering::AlphaTest alpha_test,
	                                                                     LightPass::MeshInstRendering::AlphaMode alpha_mode,
	                                                                     LightPass::MeshInstRendering::IblPreset ibl_preset,
	                                                                     LightPass::MeshInstRendering::CullMode cull_mode,
	                                                                     LightPass::MeshInstRendering::StencilMode stencil_mode,
	                                                                     Light::SourceType source_type);

	// Creates a pipeline state to apply ibl reflections.
	ut::Result<PipelineState, ut::Error> CreateMeshInstIblPassPipeline(Mesh::VertexFormat vertex_format,
	                                                                   Mesh::PolygonMode polygon_mode,
	                                                                   IblPass::MeshInstRendering::AlphaTest alpha_test,
	                                                                   IblPass::MeshInstRendering::AlphaMode alpha_mode,
	                                                                   IblPass::MeshInstRendering::CullMode cull_mode,
	                                                                   IblPass::MeshInstRendering::StencilMode stencil_mode);

	// Creates all possible lighting pipeline state permutations for a mesh instance.
	ut::Array<PipelineState> CreateMeshInstLightPassPipelinePermutations();

	// Creates all possible ibl pipeline state permutations for a mesh instance.
	ut::Array<PipelineState> CreateMeshInstIblPassPipelinePermutations();

	// Connects all descriptor sets to the corresponding shaders.
	void ConnectDescriptors();

	// Performes Z-sotring for transparent objects and stores the result in
	// @z_sorted_dc_indices array.
	void SortTransparentDrawCalls(const ut::Vector<3>& view_position,
	                              ut::Array<MeshInstance::DrawCall>& draw_list);

	// Common rendering tools.
	Toolset& tools;

	// Shaders.
	ut::Array<BoundShader> light_shader;
	ut::Array<BoundShader> ibl_shader;

	// Render pass.
	RenderPass lightpass;

	// Pipeline states.
	ut::Array<PipelineState> mesh_inst_lightpass_pipeline;
	ut::Array<PipelineState> mesh_inst_iblpass_pipeline;

	// Descriptors.
	ut::Array<LightPass::MeshInstRendering::Descriptors> lightpass_mesh_inst_desc_set;
	ut::Array<IblPass::MeshInstRendering::Descriptors> iblpass_mesh_inst_desc_set;

	// Secondary command buffers to parallelize cpu work.
	ut::Array< ut::Ref<CmdBuffer> > secondary_buffer_cache;

	// Transparent draw calls cache sorted by the distance from viewer
	ut::Array< ut::Pair<ut::uint32, float> > z_sorted_dc_indices;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
