//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/ve_render_model_batcher.h"
#include "systems/render/units/ve_render_directional_light.h"
#include "systems/render/units/ve_render_point_light.h"
#include "systems/render/units/ve_render_spot_light.h"

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
		Target diffuse;
		Target normal;
		Target depth;
		RenderPass geometry_pass;
		RenderPass light_pass;
		Framebuffer geometry_framebuffer;
		Framebuffer light_framebuffer;
		PipelineState model_pipeline;
		PipelineState light_pipeline[Light::source_type_count];
	};

	// Constructor.
	DeferredShading(Toolset& toolset);

	// Creates deferred shading (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param light_buffer - reference to the light buffer.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @return - a new DeferredShading::ViewData object or error if failed.
	ut::Result<DeferredShading::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                                Target& light_buffer,
	                                                                ut::uint32 width,
	                                                                ut::uint32 height);

	// Renders scnene to the g-buffer.
	void BakeGeometry(Context& context,
	                  Target& depth_stencil,
	                  DeferredShading::ViewData& data,
	                  Buffer& view_uniform_buffer,
	                  ModelBatcher& batcher);

	// Applies lighting techniques to the provided target.
	void Shade(Context& context,
	           DeferredShading::ViewData& data,
	           Buffer& view_uniform_buffer,
	           Light::Sources& lights);

private:
	// Renders model units to the g-buffer.
	void BakeModels(Context& context,
	                DeferredShading::ViewData& data,
	                Buffer& view_uniform_buffer,
	                ModelBatcher& batcher);

	// Creates shaders for rendering geometry to the g-buffer.
	BoundShader CreateModelGPassShader();

	// Creates a shader for the lighting pass.
	Shader CreateLightPassShader(Light::SourceType source_type);

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
	                                                             Light::SourceType source_type);

	Toolset& tools;
	BoundShader model_gpass_shader;
	Shader light_shader[Light::source_type_count];

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
