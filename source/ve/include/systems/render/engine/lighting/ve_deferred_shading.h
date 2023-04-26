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
// Encapsulates deferred shading techniques.
class DeferredShading
{
public:
	// Per-view gpu data.
	struct ViewData
	{
		// render target (can be a cubemap)
		Target diffuse;
		Target normal;
		Target depth;

		// render passes
		RenderPass geometry_pass;
		RenderPass light_pass;

		// cubemaps need a separate framebuffer for each face
		ut::Array<Framebuffer> geometry_framebuffer;
		ut::Array<Framebuffer> light_framebuffer;

		// pipeline states
		ut::Array<PipelineState> model_gpass_pipeline;
		ut::Array<PipelineState> light_pipeline;
		PipelineState ibl_pipeline;

		// secondary buffers to parallelize cpu work
		// note that cubemaps need a separate set of buffers for each face
		ut::Array<CmdBuffer> gpass_cmd;
	};

	// Constructor.
	DeferredShading(Toolset& toolset, ut::uint32 ibl_mip_count);

	// Creates deferred shading (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param light_buffer - reference to the light buffer.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @param is_cube - 'true' to create as a cubemap.
	//    @return - a new DeferredShading::ViewData object or error if failed.
	ut::Result<DeferredShading::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                                Target& light_buffer,
	                                                                ut::uint32 width,
	                                                                ut::uint32 height,
	                                                                bool is_cube);

	// Renders scnene to the g-buffer.
	void BakeOpaqueGeometry(Context& context,
	                        Target& depth_stencil,
	                        DeferredShading::ViewData& data,
	                        Buffer& view_uniform_buffer,
	                        ModelBatcher& batcher,
	                        Image::Cube::Face cubeface = Image::Cube::positive_x);

	// Applies lighting techniques to the provided target.
	void Shade(Context& context,
	           DeferredShading::ViewData& data,
	           Buffer& view_uniform_buffer,
	           Light::Sources& lights,
	           ut::Optional<Image&> ibl_cubemap,
	           Image::Cube::Face cubeface = Image::Cube::positive_x);

private:
	// Light pass pipeline permutations.
	struct LightPass
	{
		enum IblPreset
		{
			ibl_off,
			ibl_on,
			ibl_preset_count
		};

		static constexpr ut::uint32 ibl_column = 0;
		static constexpr ut::uint32 light_type_column = 1;

		typedef ut::Grid<ibl_preset_count, Light::source_type_count> Grid;
	};

	// Geometry pass pipeline and shaders permutations.
	struct GeometryPass
	{
		struct ModelRendering
		{
			enum CullMode
			{
				cull_none,
				cull_back,
				cull_mode_count
			};

			enum AlphaMode
			{
				alpha_opaque,
				alpha_test,
				alpha_mode_count
			};

			static constexpr ut::uint32 vertex_format_column = 0;
			static constexpr ut::uint32 alpha_mode_column = 1;
			static constexpr ut::uint32 cull_mode_column = 2;

			typedef ut::Grid<Mesh::vertex_format_count,
			                 GeometryPass::ModelRendering::alpha_mode_count,
			                 GeometryPass::ModelRendering::cull_mode_count> PipelineGrid;

			typedef ut::Grid<Mesh::vertex_format_count,
			                 GeometryPass::ModelRendering::alpha_mode_count> ShaderGrid;

			// Descriptor set for the model geometry pass shaders.
			struct Descriptors : public DescriptorSet
			{
				Descriptors() : DescriptorSet(view_ub, transform_ub, material_ub,
				                              sampler, diffuse, normal, material)
				{}

				Descriptor view_ub = "g_ub_view";
				Descriptor transform_ub = "g_ub_transform";
				Descriptor material_ub = "g_ub_material";
				Descriptor sampler = "g_sampler";
				Descriptor diffuse = "g_tex2d_diffuse";
				Descriptor normal = "g_tex2d_normal";
				Descriptor material = "g_tex2d_material";
			};
		};
	};

	// Descriptor set for the image based lighting pass.
	struct IblDescriptorSet : public DescriptorSet
	{
		IblDescriptorSet() : DescriptorSet(view_ub, sampler, depth,
		                                   diffuse, normal, ibl_cubemap)
		{}

		Descriptor view_ub = "g_ub_view";
		Descriptor sampler = "g_sampler";
		Descriptor depth = "g_tex2d_depth";
		Descriptor diffuse = "g_tex2d_diffuse";
		Descriptor normal = "g_tex2d_normal";
		Descriptor ibl_cubemap = "g_ibl_cubemap";
	};

	// Descriptor set for the light pass shaders.
	struct LightPassDescriptorSet : public DescriptorSet
	{
		LightPassDescriptorSet() : DescriptorSet(view_ub, light_ub, sampler,
		                                         depth, diffuse, normal)
		{}

		Descriptor view_ub = "g_ub_view";
		Descriptor light_ub = "g_ub_light";
		Descriptor sampler = "g_sampler";
		Descriptor depth = "g_tex2d_depth";
		Descriptor diffuse = "g_tex2d_diffuse";
		Descriptor normal = "g_tex2d_normal";
	};

	// Renders model units to the g-buffer.
	void BakeOpaqueModels(Context& context,
	                      DeferredShading::ViewData& data,
	                      Buffer& view_uniform_buffer,
	                      ModelBatcher& batcher,
	                      Image::Cube::Face cubeface);

	// Renders specified range of models.
	void BakeOpaqueModelsJob(Context& context,
	                         DeferredShading::ViewData& data,
	                         Buffer& view_uniform_buffer,
	                         ModelBatcher& batcher,
	                         ut::uint32 thread_id,
	                         ut::uint32 offset,
	                         ut::uint32 count);

	// Creates shaders for rendering geometry to the g-buffer.
	ut::Array<BoundShader> CreateModelGPassShader();

	// Creates a shader for the lighting pass.
	Shader CreateLightPassShader(Light::SourceType source_type,
	                             LightPass::IblPreset ibl_preset);

	// Creates a pixel shader for the image based lighting.
	Shader CreateIblShader(ut::uint32 ibl_mip_count);

	// Creates a render pass for the g-buffer.
	ut::Result<RenderPass, ut::Error> CreateModelGeometryPass(pixel::Format depth_stencil_format);

	// Creates a render pass for the shading techniques.
	ut::Result<RenderPass, ut::Error> CreateLightPass(pixel::Format depth_stencil_format,
	                                                  pixel::Format light_buffer_format);

	// Creates a pipeline state to render geometry to the g-buffer.
	ut::Result<PipelineState, ut::Error> CreateModelGPassPipeline(RenderPass& geometry_pass,
	                                                              ut::uint32 width,
	                                                              ut::uint32 height,
	                                                              Mesh::VertexFormat vertex_format,
	                                                              GeometryPass::ModelRendering::AlphaMode alpha_mode,
	                                                              GeometryPass::ModelRendering::CullMode cull_mode);

	// Creates a pipeline state to apply lighting.
	ut::Result<PipelineState, ut::Error> CreateLightPassPipeline(RenderPass& light_pass,
	                                                             ut::uint32 width,
	                                                             ut::uint32 height,
	                                                             Light::SourceType source_type,
	                                                             LightPass::IblPreset ibl_preset);

	// Creates a pipeline state to apply image based lighting.
	ut::Result<PipelineState, ut::Error> CreateIblPipeline(RenderPass& light_pass,
	                                                       ut::uint32 width,
	                                                       ut::uint32 height);

	// Connects all descriptor sets to the corresponding shaders.
	void ConnectDescriptors();

	// Common rendering tools.
	Toolset& tools;

	// Shaders.
	ut::Array<BoundShader> model_gpass_shader;
	ut::Array<Shader> light_shader;
	Shader ibl_shader;

	// Secondary command buffers to parallelize cpu work.
	ut::Array< ut::Ref<CmdBuffer> > secondary_buffer_cache;

	// Descriptors.
	ut::Array<GeometryPass::ModelRendering::Descriptors> gpass_model_desc_set;
	IblDescriptorSet ibl_desc_set;
	LightPassDescriptorSet lightpass_desc_set;

	// G-Buffer target format.
	static constexpr pixel::Format skGBufferFormat = pixel::r16g16b16a16_float;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
