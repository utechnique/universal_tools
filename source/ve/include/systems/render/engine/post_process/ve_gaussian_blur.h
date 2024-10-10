//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Highlights an area of viewport marked with stencilref_highlight stencil bit.
class GaussianBlur
{
public:
	enum Direction
	{
		direction_horizontal,
		direction_vertical,
	};

	class ViewData
	{
	public:
		ViewData(PipelineState blur_pipeline_state,
		         Buffer blur_weights_buffer,
		         const ut::Array<float>& blur_weights);

		struct BlurDescriptorSet : public DescriptorSet
		{
			BlurDescriptorSet() : DescriptorSet(ub_weights,
			                                    tex2d, sampler) {}
			Descriptor ub_weights = "g_ub_blur_weights";
			Descriptor tex2d = "g_input_texture";
			Descriptor sampler = "g_linear_sampler";
		} desc_set;

		PipelineState pipeline_state;
		Buffer weights_buffer;
		ut::Array< ut::Vector<4> > weights; // .xy - offset, .z - weight
	};

	// Constructor.
	GaussianBlur(Toolset& toolset);

	// Creates blur (per-view) data.
	//    @param render_pass - render pass that will be used for blur.
	//    @param blur_shader - reference to shader previously compiled with
	//                         @LoadShader() call.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	//    @param radius - blur radius in pixels.
	//    @param sigma - sigma (variance) parameter.
	//    @return - a new GaussianBlur::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(RenderPass& render_pass,
	                                               Shader& blur_shader,
	                                               ut::uint32 radius,
	                                               float sigma);

	// Performs gaussian blur.
	//    @param context - reference to the rendering context.
	//    @param data - reference to the GaussianBlur::ViewData object.
	//    @param fb - reference to the framebuffer
	//                (with bound destination target).
	//    @param pass - reference to the render pass with one color attachment
	//                  and no depth-stencil.
	//    @param source - reference to the source image.
	//    @param direction - blur direction (horizontal or vertical).
	void Apply(Context& context,
	           ViewData& data,
	           Framebuffer& fb,
	           RenderPass& pass,
	           Image& source,
	           Direction direction);

	// Returns compiled blur pixel shader with desired parameters.
	ut::Result<Shader, ut::Error> LoadShader(ut::uint32 radius, float sigma);

	// Calculates gaussian blur weights for the desired radius and sigma.
	static ut::Array<float> CalculateWeights(ut::uint32 radius, float sigma);

private:
	// Common rendering tools.
	Toolset& tools;
	ut::HashMap<ut::String, ut::UniquePtr<Shader> > pixel_shaders;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//