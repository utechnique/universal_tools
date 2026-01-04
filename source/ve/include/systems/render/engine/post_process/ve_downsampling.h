//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/post_process/ve_post_process_slots.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Performs 2x and 3x downsampling.
class Downsampling
{
public:
	// Defines the size of a pixel block to be downsampled
	// into one final pixel.
	enum class Granularity
	{
		s2x2, // 2x2 pixel block is downscaled to 1 pixel.
		s3x3, // 3x3 pixel block is downscaled to 1 pixel.
		count
	};

	// Sampling filter typs that can be used while downsampling.
	enum class Filter
	{
		box,
		bilinear,
		count
	};

	// Per-view downsampling data, contains uniforms and a pipeline state
	// required to perform the effect.
	class ViewData
	{
	public:
		struct DownsampleDescriptorSet : public DescriptorSet
		{
			DownsampleDescriptorSet() : DescriptorSet(ub, tex2d,
			                                          sampler) {}

			Descriptor ub = "g_ub_downsampling";
			Descriptor tex2d = "g_tex2d";
			Descriptor sampler = "g_sampler";
		} desc_set;

		struct UniformBuffer
		{
			// .xy - inverted upscale resolution
			// .zw - texcoord multipliers
			alignas(16) ut::Vector<4> parameters;
		};

		PipelineState pipeline_state;
		Buffer uniform_buffer;
		Granularity granularity;
		Filter filter;
		ut::Vector<2, float> original_size;
		ut::Vector<2, float> downsampled_size;
	};

	// Constructor. Saves references to the rendering tools and render
	// passes required to perform downsampling.
	Downsampling(Toolset& toolset,
	             RenderPass& color_only_pass);

	// Creates downsampling (per-view) data.
	//    @param original_width - width of the original image
	//                            in pixels before downsampling.
	//    @param original_height - height of the original image
	//                             in pixels before downsampling.
	//    @param granularity - downsampling granularity.
	//    @param filter - downsampling filter.
	//    @return - a new Downsampling::ViewData object or error if failed.
	ut::Result<ViewData, ut::Error> CreateViewData(ut::uint32 original_width,
	                                               ut::uint32 original_height,
	                                               Granularity granularity,
	                                               Filter filter);

	// Performs downsampling.
	//    @param data - per-view data containing downsampling information.
	//    @param context - a reference to the rendering context.
	//    @param framebuffer - a reference to the framebuffer to
	//                         be used for rendering.
	//    @param source - a reference to the image to be downsampled.
	//    @param texcoord_factor - texture coordinates of the full screen quad
	//                             will be multiplied by this value in shader.
	void Apply(ViewData& data,
	           Context& context,
	           Framebuffer& framebuffer,
	           Image& source,
	           const ut::Vector<2, float>& texcoord_factor = ut::Vector<2, float>(1.0f));

	// Return the size of the buffer after downsampling.
	//    @param original_size - size of the buffer before downsampling.
	//    @param granularity - downsampling granularity.
	//    @return - size of the buffer after downsampling.
	static ut::Vector<2, ut::uint32> CalculateDownscaledSize(const ut::Vector<2, ut::uint32>& original_size,
	                                                         Granularity granularity);

private:
	// This grid defines downsampling pixel shader permutations.
	typedef ut::Grid<static_cast<size_t>(Granularity::count),
	                 static_cast<size_t>(Filter::count)> Grid;

	// Column IDs for the shader permutation grid.
	static constexpr ut::uint32 granularity_column = 0;
	static constexpr ut::uint32 filter_column = 1;

	// Returns a set of compiled pixel shader permutations
	// performing the downsampling effect.
	ut::Array<Shader> LoadShaders();

	// Creates a pipeline state for the desired downsampling effect.
	ut::Result<PipelineState, ut::Error> CreatePipelineState(Granularity granularity,
	                                                         Filter filter);

	// Common rendering tools.
	Toolset& tools;

	// Full screen 2D quad.
	Mesh::Subset& fullscreen_quad;

	// Downsampling rendering pass uses only one color
	// target in a framebuffer without depth-stencil.
	RenderPass& color_only_pass;

	// Pixel shader permutations.
	ut::Array<Shader> shaders;

	// Custom viewports are needed if the size of render target doesn't match
	// the size of the expected downsampled buffer. Only a part of the render
	// target will be used in this case.
	ut::Array<Viewport> viewports;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//