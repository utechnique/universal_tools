//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/ve_render_model_batcher.h"
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
// can't be rendered via deferred rendering (such as transparent models).
class ForwardShading
{
public:
	// Per-view gpu data.
	struct ViewData
	{
		RenderPass lightpass;
		ut::Array<Framebuffer> light_framebuffer;
		ut::Array<PipelineState> lightpass_pipeline;
		ut::Array<PipelineState> iblpass_pipeline;

		// secondary buffers to parallelize cpu work
		// note that cubemaps need a separate set of buffers for each face
		ut::Array<CmdBuffer> lightpass_cmd;
	};

	// Constructor.
	ForwardShading(Toolset& toolset, ut::uint32 ibl_mip_count);

	// Creates deferred shading (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param light_buffer - reference to the light buffer.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @param is_cube - 'true' to create as a cubemap.
	//    @return - a new ForwardShading::ViewData object or error if failed.
	ut::Result<ForwardShading::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                               Target& light_buffer,
	                                                               ut::uint32 width,
	                                                               ut::uint32 height,
	                                                               bool is_cube);

	// Renders scene directly to the light buffer.
	void DrawTransparentGeometry(Context& context,
	                             ForwardShading::ViewData& data,
	                             Buffer& view_uniform_buffer,
	                             ModelBatcher& batcher,
	                             Light::Sources& lights,
	                             ut::Optional<Image&> ibl_cubemap,
	                             Image::Cube::Face cubeface = Image::Cube::positive_x);

private:
	// Light pass pipeline permutations.
	struct LightPass
	{
		enum AlphaMode
		{
			alpha_blend,
			alpha_add,
			alpha_mode_count
		};

		enum IblPreset
		{
			ibl_off,
			ibl_on,
			ibl_preset_count
		};

		enum CullMode
		{
			cull_front,
			cull_back,
			cull_mode_count
		};

		static constexpr ut::uint32 vertex_format_column = 0;
		static constexpr ut::uint32 ibl_column = 1;
		static constexpr ut::uint32 light_type_column = 2;
		static constexpr ut::uint32 alpha_mode_column = 3;
		static constexpr ut::uint32 cull_mode_column = 4;

		typedef ut::Grid<Mesh::vertex_format_count,
		                 ibl_preset_count,
		                 Light::source_type_count,
		                 alpha_mode_count,
		                 cull_mode_count> PipelineGrid;

		typedef ut::Grid<Mesh::vertex_format_count,
		                 ibl_preset_count,
		                 Light::source_type_count> ShaderGrid;
	};

	// IBL pass pipeline permutations.
	struct IblPass
	{
		enum AlphaMode
		{
			alpha_blend,
			alpha_add,
			alpha_mode_count
		};

		enum CullMode
		{
			cull_front,
			cull_back,
			cull_mode_count
		};

		static constexpr ut::uint32 vertex_format_column = 0;
		static constexpr ut::uint32 alpha_mode_column = 1;
		static constexpr ut::uint32 cull_mode_column = 2;

		typedef ut::Grid<Mesh::vertex_format_count,
		                 alpha_mode_count,
		                 cull_mode_count> PipelineGrid;

		typedef ut::Grid<Mesh::vertex_format_count> ShaderGrid;
	};

	// Descriptor set for the light pass shaders.
	struct LightPassDescriptorSet : public DescriptorSet
	{
		LightPassDescriptorSet() : DescriptorSet(view_ub, light_ub, transform_ub,
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

	// Descriptor set for the ibl pass shaders.
	struct IblPassDescriptorSet : public DescriptorSet
	{
		IblPassDescriptorSet() : DescriptorSet(view_ub, transform_ub,
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

	// Renders specified range of models.
	void RenderTransparentModelJob(Context& context,
	                               ForwardShading::ViewData& data,
	                               Light::Sources& lights,
	                               Buffer& view_uniform_buffer,
	                               ModelBatcher& batcher,
	                               ut::Optional<Image&> ibl_cubemap,
	                               ut::uint32 thread_id,
	                               ut::uint32 offset,
	                               ut::uint32 count);

	// Applies direct lighting to the specified model.
	void RenderTransparentModelLights(Context& context,
	                                  ForwardShading::ViewData& data,
	                                  Light::Sources& lights,
	                                  Buffer& view_uniform_buffer,
	                                  ModelBatcher& batcher,
	                                  LightPass::IblPreset ibl_preset,
	                                  LightPass::CullMode cull_mode,
	                                  LightPass::AlphaMode alpha_mode,
	                                  ut::uint32 drawcall_id,
	                                  ut::uint32 thread_id);

	// Applies direct lighting to the specified model.
	void RenderTransparentModelIbl(Context& context,
	                               ForwardShading::ViewData& data,
	                               Image& ibl_cubemap,
	                               Buffer& view_uniform_buffer,
	                               ModelBatcher& batcher,
	                               IblPass::CullMode cull_mode,
	                               IblPass::AlphaMode alpha_mode,
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
	ut::Array<BoundShader> CreateLightPassShader();

	// Creates a shader for the ibl pass.
	ut::Array<BoundShader> CreateIblShader(ut::uint32 ibl_mip_count);

	// Creates a render pass for the shading techniques.
	ut::Result<RenderPass, ut::Error> CreateLightPass(pixel::Format depth_stencil_format,
	                                                  pixel::Format light_buffer_format);

	// Creates a pipeline state to apply lighting.
	ut::Result<PipelineState, ut::Error> CreateLightPassPipeline(RenderPass& light_pass,
	                                                             ut::uint32 width,
	                                                             ut::uint32 height,
	                                                             Mesh::VertexFormat vertex_format,
	                                                             LightPass::AlphaMode alpha_mode,
	                                                             LightPass::IblPreset ibl_preset,
	                                                             Light::SourceType source_type,
	                                                             LightPass::CullMode cull_mode);

	// Creates a pipeline state to apply ibl reflections.
	ut::Result<PipelineState, ut::Error> CreateIblPassPipeline(RenderPass& light_pass,
	                                                           ut::uint32 width,
	                                                           ut::uint32 height,
	                                                           Mesh::VertexFormat vertex_format,
	                                                           IblPass::AlphaMode alpha_mode,
	                                                           IblPass::CullMode cull_mode);

	// Connects all descriptor sets to the corresponding shaders.
	void ConnectDescriptors();

	// Common rendering tools.
	Toolset& tools;

	// Shaders.
	ut::Array<BoundShader> light_shader;
	ut::Array<BoundShader> ibl_shader;

	// Descriptors.
	ut::Array<LightPassDescriptorSet> lightpass_desc_set;
	ut::Array<IblPassDescriptorSet> iblpass_desc_set;

	// Secondary command buffers to parallelize cpu work.
	ut::Array< ut::Ref<CmdBuffer> > secondary_buffer_cache;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//