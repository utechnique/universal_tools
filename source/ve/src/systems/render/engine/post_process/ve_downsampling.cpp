//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_downsampling.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Constructor. Saves references to the rendering tools and render
// passes required to perform downsampling.
Downsampling::Downsampling(Toolset& toolset,
                           RenderPass& in_color_only_pass) : tools(toolset)
                                                           , fullscreen_quad(tools.rc_mgr.fullscreen_quad->subsets.GetFirst())
                                                           , color_only_pass(in_color_only_pass)
                                                           , shaders(LoadShaders())
{
	Viewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 0.0f;
	viewport.height = 0.0f;
	viewport.min_depth = 0.0f;
	viewport.max_depth = 1.0f;
	viewports.Add(viewport);
}

// Creates downsampling (per-view) data.
//    @param original_width - width of the original image
//                            in pixels before downsampling.
//    @param original_height - height of the original image
//                             in pixels before downsampling.
//    @param granularity - downsampling granularity.
//    @param filter - downsampling filter.
//    @return - a new Downsampling::ViewData object or error if failed.
ut::Result<Downsampling::ViewData, ut::Error> Downsampling::CreateViewData(ut::uint32 original_width,
                                                                           ut::uint32 original_height,
                                                                           Granularity granularity,
                                                                           Filter filter)
{
	// create uniform buffer
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::Type::uniform;
	buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
	buffer_info.size = sizeof(ViewData::UniformBuffer);
	ut::Result<Buffer, ut::Error> uniform_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!uniform_buffer)
	{
		return ut::MakeError(uniform_buffer.MoveAlt());
	}

	// create pipeline state
	ut::Result<PipelineState, ut::Error> pipeline = CreatePipelineState(granularity,
	                                                                    filter);
	if (!pipeline)
	{
		return ut::MakeError(pipeline.MoveAlt());
	}

	// initialize size
	const ut::Vector<2, ut::uint32> original_size(original_width,
	                                              original_height);
	const ut::Vector<2, ut::uint32> downsampled_size = CalculateDownscaledSize(original_size,
	                                                                           granularity);

	// create final data object
	ViewData data =
	{
		ViewData::DownsampleDescriptorSet(),
		pipeline.Move(),
		uniform_buffer.Move(),
		granularity,
		filter,
		ut::Vector<2, float>(original_size.X(), original_size.Y()),
		ut::Vector<2, float>(downsampled_size.X(), downsampled_size.Y())
	};

	// connect descriptors
	data.desc_set.Connect(shaders.GetFirst());

	// success
	return data;
}

// Performs downsampling.
//    @param data - per-view data containing downsampling information.
//    @param context - a reference to the rendering context.
//    @param framebuffer - a reference to the framebuffer to
//                         be used for rendering.
//    @param source - a reference to the image to be downsampled.
//    @param texcoord_factor - texture coordinates of the full screen quad
//                             will be multiplied by this value in shader.
void Downsampling::Apply(ViewData& data,
                         Context& context,
                         Framebuffer& framebuffer,
                         Image& source,
                         const ut::Vector<2, float>& texcoord_factor)
{
	const Framebuffer::Info& fb_info = framebuffer.GetInfo();

	// update viewport
	Viewport& viewport = viewports.GetFirst();
	viewport.width = data.downsampled_size.X();
	viewport.height = data.downsampled_size.Y();

	// update uniform buffer
	ViewData::UniformBuffer uniforms;
	uniforms.parameters.X() = 1.0f / data.original_size.X();
	uniforms.parameters.Y() = 1.0f / data.original_size.Y();
	uniforms.parameters.Z() = texcoord_factor.X();
	uniforms.parameters.W() = texcoord_factor.Y();
	ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                                                    data.uniform_buffer,
	                                                                    &uniforms);
	if (update_ub_error)
	{
		throw update_ub_error.Move();
	}

	// set shader resources
	data.desc_set.ub.BindUniformBuffer(data.uniform_buffer);
	data.desc_set.tex2d.BindImage(source);
	data.desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);

	// draw quad
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(color_only_pass,
	                        framebuffer,
	                        render_area,
	                        ut::Color<4>(0), 1.0f);
	context.BindPipelineState(data.pipeline_state, viewports);
	context.BindDescriptorSet(data.desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->subsets.GetFirst().vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);
	context.EndRenderPass();
}

// Return the size of the buffer after downsampling.
	//    @param original_size - size of the buffer before downsampling.
	//    @param granularity - downsampling granularity.
	//    @return - size of the buffer after downsampling.
ut::Vector<2, ut::uint32> Downsampling::CalculateDownscaledSize(const ut::Vector<2, ut::uint32>& original_size,
                                                                Granularity granularity)
{
	switch (granularity)
	{
	case Granularity::s2x2: return original_size.ElementWise() / 2;
	case Granularity::s3x3: return original_size.ElementWise() / 3;
	default: return original_size;
	}
}

// Returns a set of compiled pixel shader permutations
	// performing the downsampling effect.
ut::Array<Shader> Downsampling::LoadShaders()
{
	ut::Array<Shader> shaders;

	constexpr size_t permutation_count = Grid::size;
	for (size_t i = 0; i < permutation_count; i++)
	{
		Shader::Macros macros;

		// granularity
		const Granularity granularity =
			static_cast<Granularity>(Grid::GetCoordinate<granularity_column>(i));
		Shader::MacroDefinition granularity_macro;
		granularity_macro.name = granularity == Granularity::s3x3 ?
		                         "GRANULARITY_3x3" :
		                         "GRANULARITY_2x2";
		granularity_macro.value = "1";
		macros.Add(ut::Move(granularity_macro));
		const ut::String granularity_suffix = granularity == Granularity::s3x3 ? "3x3" : "2x2";

		// filter
		const Filter filter = static_cast<Filter>(Grid::GetCoordinate<filter_column>(i));
		Shader::MacroDefinition filter_macro;
		filter_macro.name = filter == Filter::bilinear ?
		                    "FILTER_BILINEAR" :
		                    "FILTER_BOX";
		filter_macro.value = "1";
		macros.Add(ut::Move(filter_macro));
		const ut::String filter_suffix = filter == Filter::bilinear ? "bilinear" : "box";

		// shader permutation name
		ut::String shader_name = "downsampling_ps_";
		shader_name += granularity_suffix + "_" + filter_suffix;

		shaders.Add(tools.shader_loader.Load(Shader::Stage::pixel,
		                                     ut::Move(shader_name),
		                                     "PS",
		                                     "downsampling.hlsl",
		                                     macros).MoveOrThrow());
	}

	return shaders;
}

// Creates a pipeline state for the desired downsampling effect.
ut::Result<PipelineState, ut::Error> Downsampling::CreatePipelineState(Granularity granularity,
                                                                       Filter filter)
{
	const size_t shader_id = Grid::GetId(static_cast<size_t>(granularity),
	                                     static_cast<size_t>(filter));

	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, shaders[shader_id]);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(ut::Move(info), color_only_pass);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//