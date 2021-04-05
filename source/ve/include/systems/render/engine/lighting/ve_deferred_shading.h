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

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Encapsulates deferred shading techniques.
class DeferredShading
{
public:
	enum IblPreset
	{
		ibl_off,
		ibl_on,
		ibl_preset_count
	};

	// Per-view gpu data.
	struct ViewData
	{
		Target diffuse;
		Target normal;
		Target depth;
		RenderPass geometry_pass;
		RenderPass light_pass;
		ut::Array<Framebuffer> geometry_framebuffer;
		ut::Array<Framebuffer> light_framebuffer;
		PipelineState model_pipeline;
		PipelineState light_pipeline[ibl_preset_count][Light::source_type_count];
		PipelineState ibl_pipeline;
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
	void BakeGeometry(Context& context,
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
	// Renders model units to the g-buffer.
	void BakeModels(Context& context,
	                DeferredShading::ViewData& data,
	                Buffer& view_uniform_buffer,
	                ModelBatcher& batcher,
	                Image::Cube::Face cubeface);

	// Creates shaders for rendering geometry to the g-buffer.
	BoundShader CreateModelGPassShader();

	// Creates a shader for the lighting pass.
	Shader CreateLightPassShader(Light::SourceType source_type, bool ibl_on);

	// Creates a pixel shader for the image based lighting.
	Shader CreateIblShader(ut::uint32 ibl_mip_count);

	// Creates a render pass for the g-buffer.
	ut::Result<RenderPass, ut::Error> CreateGeometryPass(pixel::Format depth_stencil_format);

	// Creates a render pass for the shading techniques.
	ut::Result<RenderPass, ut::Error> CreateLightPass(pixel::Format depth_stencil_format,
	                                                  pixel::Format light_buffer_format);

	// Creates a pipeline state to render geometry to the g-buffer.
	ut::Result<PipelineState, ut::Error> CreateModelGPassPipeline(RenderPass& geometry_pass,
	                                                              ut::uint32 width,
	                                                              ut::uint32 height);

	// Creates a pipeline state to apply lighting.
	ut::Result<PipelineState, ut::Error> CreateLightPassPipeline(RenderPass& light_pass,
	                                                             ut::uint32 width,
	                                                             ut::uint32 height,
	                                                             Light::SourceType source_type,
	                                                             IblPreset ibl_preset);

	// Creates a pipeline state to apply image based lighting.
	ut::Result<PipelineState, ut::Error> CreateIblPipeline(RenderPass& light_pass,
	                                                       ut::uint32 width,
	                                                       ut::uint32 height);

	Toolset& tools;
	BoundShader model_gpass_shader;
	Shader light_shader[ibl_preset_count][Light::source_type_count];
	Shader ibl_shader;

	struct GPassModelDescriptorSet : public DescriptorSet
	{
		GPassModelDescriptorSet() : DescriptorSet(view_ub, transform_ub, material_ub,
		                                          sampler, diffuse, normal, material)
		{}

		Descriptor view_ub = "g_ub_view";
		Descriptor transform_ub = "g_ub_transform";
		Descriptor material_ub = "g_ub_material";
		Descriptor sampler = "g_sampler";
		Descriptor diffuse = "g_tex2d_diffuse";
		Descriptor normal = "g_tex2d_normal";
		Descriptor material = "g_tex2d_material";
	} gpass_desc_set;

	struct IblDescriptorSet : public DescriptorSet
	{
		IblDescriptorSet() : DescriptorSet(view_ub, sampler, depth, ibl_sampler,
		                                   diffuse, normal, ibl_cubemap)
		{}

		Descriptor view_ub = "g_ub_view";
		Descriptor sampler = "g_sampler";
		Descriptor ibl_sampler = "g_ibl_sampler";
		Descriptor depth = "g_tex2d_depth";
		Descriptor diffuse = "g_tex2d_diffuse";
		Descriptor normal = "g_tex2d_normal";
		Descriptor ibl_cubemap = "g_ibl_cubemap";
	} ibl_desc_set;

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
	} lightpass_desc_set;

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
