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

	// Renders units directly to the light buffer.
	void Draw(Context& context,
	          ForwardShading::ViewData& data,
	          Buffer& view_uniform_buffer,
	          const ut::Vector<3>& view_position,
	          Batcher& batcher,
	          Light::Sources& lights,
	          ut::Optional<Image&> ibl_cubemap,
	          Image::Cube::Face cubeface = Image::Cube::Face::positive_x);

private:
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
			overwrite,
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

		enum class IblPreset
		{
			off,
			on,
			count
		};

		struct PipelineGrid
		{
			static constexpr ut::uint32 vertex_format_column = 0;
			static constexpr ut::uint32 alpha_test_column = 1;
			static constexpr ut::uint32 alpha_mode_column = 2;
			static constexpr ut::uint32 cull_mode_column = 3;
			static constexpr ut::uint32 stencil_mode_column = 4;
			static constexpr ut::uint32 polygon_mode_column = 5;
			static constexpr ut::uint32 ibl_column = 6;
			static constexpr ut::uint32 light_type_column = 7;
		};

		struct ShaderGrid
		{
			static constexpr ut::uint32 vertex_format_column = 0;
			static constexpr ut::uint32 alpha_test_column = 1;
			static constexpr ut::uint32 ibl_column = 2;
			static constexpr ut::uint32 light_type_column = 3;
		};

		struct MeshDescriptors : public DescriptorSet
		{
			template<typename... DescriptorTypes>
			MeshDescriptors(DescriptorTypes&... descriptors) :
				DescriptorSet(descriptors...)
			{}

			Descriptor view_ub = "g_ub_view";
			Descriptor light_ub = "g_ub_light";
			Descriptor transform_ub = "g_ub_transform";
			Descriptor material_ub = "g_ub_material";
			Descriptor sampler = "g_sampler";
			Descriptor base_color = "g_tex2d_base_color";
			Descriptor normal = "g_tex2d_normal";
			Descriptor metallic_roughness = "g_tex2d_metallic_roughness";
			Descriptor ibl_cubemap = "g_ibl_cubemap";
			Descriptor occlusion = "g_tex2d_occlusion";
			Descriptor emissive = "g_tex2d_emissive";
		};

		// IBL pass pipeline permutations.
		struct IblPass
		{
			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count),
			                 static_cast<size_t>(AlphaMode::count),
			                 static_cast<size_t>(CullMode::count),
			                 static_cast<size_t>(StencilMode::count),
			                 static_cast<size_t>(Mesh::PolygonMode::count)> PipelineGrid;

			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count)> ShaderGrid;

			struct Descriptors : public MeshDescriptors
			{
				Descriptors() : MeshDescriptors(view_ub, transform_ub,
				                                material_ub, sampler,
				                                base_color, normal,
				                                metallic_roughness,
				                                ibl_cubemap)
				{}
			};
		};

		// Light pass pipeline permutations.
		struct LightPass
		{
			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count),
			                 static_cast<size_t>(AlphaMode::count),
			                 static_cast<size_t>(CullMode::count),
			                 static_cast<size_t>(StencilMode::count),
			                 static_cast<size_t>(Mesh::PolygonMode::count),
			                 static_cast<size_t>(IblPreset::count),
			                 static_cast<size_t>(Light::SourceType::count)> PipelineGrid;

			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count),
			                 static_cast<size_t>(IblPreset::count),
			                 static_cast<size_t>(Light::SourceType::count)> ShaderGrid;

			// Descriptor set for the light pass shaders.
			struct DirectLightDescriptors : public MeshDescriptors
			{
				DirectLightDescriptors() : MeshDescriptors(view_ub, light_ub, transform_ub,
				                                           material_ub, sampler, base_color,
				                                           normal, metallic_roughness)
				{}
			};

			// Descriptor set for the ambient pass shaders.
			struct AmbientLightDescriptors : public MeshDescriptors
			{
				enum class IblOn { select };
				enum class IblOff { select };

				AmbientLightDescriptors(IblOn) : MeshDescriptors(view_ub, light_ub, transform_ub,
				                                                 material_ub, sampler, base_color,
				                                                 metallic_roughness, occlusion)
				{}

				AmbientLightDescriptors(IblOff) : MeshDescriptors(view_ub, light_ub, transform_ub,
				                                                  material_ub, sampler, base_color,
				                                                  occlusion)
				{}
			};
		};

		// Emissive pass pipeline permutations.
		struct EmissivePass
		{
			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count),
			                 static_cast<size_t>(AlphaMode::count),
			                 static_cast<size_t>(CullMode::count),
			                 static_cast<size_t>(StencilMode::count),
			                 static_cast<size_t>(Mesh::PolygonMode::count)> PipelineGrid;

			typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
			                 static_cast<size_t>(AlphaTest::count)> ShaderGrid;

			struct Descriptors : public MeshDescriptors
			{
				enum class AlphaTestOn { select };
				enum class AlphaTestOff { select };

				Descriptors(AlphaTestOff) : MeshDescriptors(view_ub, transform_ub,
				                                            material_ub, sampler,
				                                            emissive)
				{}
				
				Descriptors(AlphaTestOn) : MeshDescriptors(view_ub, transform_ub,
				                                           material_ub, sampler,
				                                           base_color, emissive)
				{}
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
	void RenderTransparentMeshInstanceIbl(Context& context,
	                                      ForwardShading::ViewData& data,
	                                      Image& ibl_cubemap,
	                                      Buffer& view_uniform_buffer,
	                                      Batcher& batcher,
	                                      MeshInstRendering::CullMode cull_mode,
	                                      MeshInstRendering::AlphaMode alpha_mode,
	                                      ut::uint32 drawcall_id,
	                                      ut::uint32 thread_id);

	// Applies direct lighting to the specified mesh instance.
	void RenderTransparentMeshInstanceLights(Context& context,
	                                         ForwardShading::ViewData& data,
	                                         Light::Sources& lights,
	                                         Buffer& view_uniform_buffer,
	                                         Batcher& batcher,
	                                         MeshInstRendering::IblPreset ibl_preset,
	                                         MeshInstRendering::CullMode cull_mode,
	                                         MeshInstRendering::AlphaMode alpha_mode,
	                                         ut::uint32 drawcall_id,
	                                         ut::uint32 thread_id);

	// Applies emissive lighting to the specified mesh instance.
	void RenderTransparentMeshInstanceEmissive(Context& context,
	                                           ForwardShading::ViewData& data,
	                                           Buffer& view_uniform_buffer,
	                                           Batcher& batcher,
	                                           MeshInstRendering::CullMode cull_mode,
	                                           MeshInstRendering::AlphaMode alpha_mode,
	                                           ut::uint32 drawcall_id,
	                                           ut::uint32 thread_id);

	// Renders provided mesh.
	static void DrawMesh(Context& context,
	                     Mesh::Subset& mesh_subset,
	                     PipelineState& pipeline,
	                     DescriptorSet& desc_set,
	                     Buffer& instance_buffer,
	                     ut::uint32 instance_offset);

	// Performes Z-sotring for transparent objects and stores the result in
	// @z_sorted_dc_indices array.
	void SortTransparentDrawCalls(const ut::Vector<3>& view_position,
	                              ut::Array<MeshInstance::DrawCall>& draw_list);

	// Creates a render pass for the shading techniques.
	RenderPass CreateLightPass();

	// Creates a shader for the ibl pass.
	ut::Array<BoundShader> CreateMeshInstIblShader(ut::uint32 ibl_mip_count);

	// Creates a shader for the lighting pass.
	ut::Array<BoundShader> CreateMeshInstLightPassShader();

	// Creates a shader for the emissive pass.
	ut::Array<BoundShader> CreateMeshInstEmissivePassShader();

	// Creates a pipeline state to apply ibl reflections.
	ut::Result<PipelineState, ut::Error> CreateMeshInstIblPassPipeline(Mesh::VertexFormat vertex_format,
	                                                                   Mesh::PolygonMode polygon_mode,
	                                                                   MeshInstRendering::AlphaTest alpha_test,
	                                                                   MeshInstRendering::AlphaMode alpha_mode,
	                                                                   MeshInstRendering::CullMode cull_mode,
	                                                                   MeshInstRendering::StencilMode stencil_mode);

	// Creates a pipeline state to apply lighting.
	ut::Result<PipelineState, ut::Error> CreateMeshInstLightPassPipeline(Mesh::VertexFormat vertex_format,
	                                                                     Mesh::PolygonMode polygon_mode,
	                                                                     MeshInstRendering::AlphaTest alpha_test,
	                                                                     MeshInstRendering::AlphaMode alpha_mode,
	                                                                     MeshInstRendering::CullMode cull_mode,
	                                                                     MeshInstRendering::StencilMode stencil_mode,
	                                                                     MeshInstRendering::IblPreset ibl_preset,
	                                                                     Light::SourceType source_type);

	// Creates a pipeline state to apply emissive color.
	ut::Result<PipelineState, ut::Error> CreateMeshInstEmissivePassPipeline(Mesh::VertexFormat vertex_format,
	                                                                       Mesh::PolygonMode polygon_mode,
	                                                                       MeshInstRendering::AlphaTest alpha_test,
	                                                                       MeshInstRendering::AlphaMode alpha_mode,
	                                                                       MeshInstRendering::CullMode cull_mode,
	                                                                       MeshInstRendering::StencilMode stencil_mode);

	// Creates all possible ibl pipeline state permutations for a mesh instance.
	ut::Array<PipelineState> CreateMeshInstIblPassPipelinePermutations();

	// Creates all possible lighting pipeline state permutations for a mesh instance.
	ut::Array<PipelineState> CreateMeshInstLightPassPipelinePermutations();

	// Creates all possible emissive pipeline state permutations for a mesh instance.
	ut::Array<PipelineState> CreateMeshInstEmissivePassPipelinePermutations();

	// Connects all descriptor sets to the corresponding shaders.
	void ConnectDescriptors();

	// Common rendering tools.
	Toolset& tools;

	// Render pass.
	RenderPass lightpass;

	// Shaders.
	ut::Array<BoundShader> ibl_shader;
	ut::Array<BoundShader> light_shader;
	ut::Array<BoundShader> emissive_shader;

	// Pipeline states.
	ut::Array<PipelineState> ibl_pass_pipeline;
	ut::Array<PipelineState> light_pass_pipeline;
	ut::Array<PipelineState> emissive_pass_pipeline;

	// Descriptors.
	ut::Array<MeshInstRendering::IblPass::Descriptors> ibl_pass_desc_set;
	ut::Array<MeshInstRendering::LightPass::DirectLightDescriptors> light_pass_desc_set;
	ut::Array<MeshInstRendering::LightPass::AmbientLightDescriptors> ambient_pass_desc_set[static_cast<size_t>(MeshInstRendering::IblPreset::count)];
	ut::Array<MeshInstRendering::EmissivePass::Descriptors> emissive_pass_desc_set[static_cast<size_t>(MeshInstRendering::AlphaTest::count)];

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
