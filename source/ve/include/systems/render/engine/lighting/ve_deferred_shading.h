//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/ve_render_model_batcher.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//

class DeferredShading
{
public:
	struct ViewData
	{
		// descriptor set for quad shader
		struct GPassModelDescriptorSet : public DescriptorSet
		{
			GPassModelDescriptorSet() : DescriptorSet(view_ub,
			                                          transform_ub,
			                                          material_ub,
			                                          sampler,
			                                          diffuse,
			                                          normal,
			                                          material)
			{}

			Descriptor view_ub = "g_ub_view";
			Descriptor transform_ub = "g_ub_transform";
			Descriptor material_ub = "g_ub_material";
			Descriptor sampler = "g_sampler";
			Descriptor diffuse = "g_tex2d_diffuse";
			Descriptor normal = "g_tex2d_normal";
			Descriptor material = "g_tex2d_material";
		};

		Target diffuse;
		Target normal;
		RenderPass pass;
		Framebuffer framebuffer;
		PipelineState model_pipeline;
		GPassModelDescriptorSet geometry_pass_desc_set;
	};

	// Constructor.
	DeferredShading(Toolset& toolset);

	// Creates deferred shading (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @return - a new DeferredShading::ViewData object or error if failed.
	ut::Result<DeferredShading::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                                ut::uint32 width,
	                                                                ut::uint32 height);

	// Renders model units to the g-buffer.
	void BakeModels(Context& context,
	                DeferredShading::ViewData& data,
	                Buffer& view_uniform_buffer,
	                ModelBatcher& batcher);

private:
	// Creates shaders for rendering geometry to the g-buffer.
	BoundShader CreateModelGPassShader();

	// Creates a render pass for the g-buffer.
	ut::Result<RenderPass, ut::Error> CreateGeometryPass(pixel::Format depth_stencil_format);

	// Creates a pipeline state to render geometry to the g-buffer.
	ut::Result<PipelineState, ut::Error> CreateModelGPassPipeline(RenderPass& render_pass,
	                                                              ut::uint32 width,
	                                                              ut::uint32 height);

	Toolset& tools;
	BoundShader model_gpass_shader;

	// G-Buffer target format.
	static constexpr pixel::Format skGBufferFormat = pixel::r8g8b8a8_unorm;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
