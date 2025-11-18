//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_gaussian_blur.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Gaussian blur (per-view) data constructor.
GaussianBlur::ViewData::ViewData(PipelineState in_pipeline,
                                 Buffer in_weight_buffer,
                                 const ut::Array<float>& in_weights) : pipeline_state(ut::Move(in_pipeline))
                                                                     , weights_buffer(ut::Move(in_weight_buffer))
                                                                     , weights(in_weights.Count())
{
	for (size_t i = 0; i < in_weights.Count(); i++)
	{
		weights[i].Z() = in_weights[i];
	}
}

// Gaussian blur constructor.
GaussianBlur::GaussianBlur(Toolset& toolset) : tools(toolset)
                                             , fullscreen_quad(tools.rc_mgr.fullscreen_quad->subsets.GetFirst())
{}

// Creates blur (per-view) data.
//    @param render_pass - render pass that will be used for blur.
//    @param blur_shader - reference to shader previously compiled with
//                         @LoadShader() call.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @param radius - blur radius in pixels.
//    @param sigma - sigma (variance) parameter.
//    @return - a new GaussianBlur::ViewData object or error if failed.
ut::Result<GaussianBlur::ViewData, ut::Error> GaussianBlur::CreateViewData(RenderPass& render_pass,
                                                                           Shader& blur_shader,
                                                                           ut::uint32 radius,
                                                                           float sigma)
{
	// create pipeline state
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.shaders.quad_vs);
	info.SetShader(Shader::Stage::pixel, blur_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	ut::Result<PipelineState, ut::Error> pipeline_state = tools.device.CreatePipelineState(ut::Move(info),
	                                                                                       render_pass);
	if (!pipeline_state)
	{
		return ut::MakeError(pipeline_state.MoveAlt());
	}

	// calculate weights and prepare buffer data
	ut::Array<float> weights = CalculateWeights(radius, sigma);

	// create the uniform buffer for weights
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::Type::uniform;
	buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
	buffer_info.size = weights.Count() * sizeof(ut::Vector<4>);
	ut::Result<Buffer, ut::Error> weights_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!weights_buffer)
	{
		throw ut::Error(weights_buffer.MoveAlt());
	}

	// create final data object
	GaussianBlur::ViewData data(pipeline_state.Move(),
	                            weights_buffer.Move(),
	                            weights);

	// connect descriptors
	data.desc_set.Connect(blur_shader);

	// success
	return data;
}

// Performs gaussian blur.
//    @param context - reference to the rendering context.
//    @param data - reference to the GaussianBlur::ViewData object.
//    @param fb - reference to the framebuffer
//                (with bound destination target).
//    @param pass - reference to the render pass with one color attachment
//                  and no depth-stencil.
//    @param source - reference to the source image.
//    @param direction - blur direction (horizontal or vertical).
void GaussianBlur::Apply(Context& context,
                         ViewData& data,
                         Framebuffer& fb,
                         RenderPass& pass,
                         Image& source,
                         Direction direction)
{
	const Framebuffer::Info& fb_info = fb.GetInfo();

	// calculate texel offset
	const ut::int32 radius = static_cast<ut::int32>((data.weights.Count() - 1) / 2);
	for (ut::int32 i = -radius; i <= radius; ++i)
	{
		ut::Vector<4>& weight = data.weights[i + radius];
		
		if (direction == GaussianBlur::Direction::horizontal)
		{
			weight.X() = static_cast<float>(i) / fb_info.width;
			weight.Y() = 0.0f;
		}
		else
		{
			weight.X() = 0.0f;
			weight.Y() = static_cast<float>(i) / fb_info.height;
		}
	}

	// update uniform buffer
	ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                                                    data.weights_buffer,
	                                                                    data.weights.GetAddress());
	if (update_ub_error)
	{
		throw update_ub_error.Move();
	}

	// set shader resources
	data.desc_set.ub_weights.BindUniformBuffer(data.weights_buffer);
	data.desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);
	data.desc_set.tex2d.BindImage(source);

	// draw quad
	const ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(pass, fb,
	                        render_area,
	                        ut::Color<4>(0), 1.0f);
	context.BindPipelineState(data.pipeline_state);
	context.BindDescriptorSet(data.desc_set);
	context.BindVertexBuffer(fullscreen_quad.vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);
	context.EndRenderPass();
}

// Returns compiled blur pixel shader with desired parameters.
ut::Result<Shader, ut::Error> GaussianBlur::LoadShader(ut::uint32 radius,
                                                       float sigma)
{
	const ut::String radius_str = ut::Print(radius);

	// accept only 2 digits after '.' for sigma value
	ut::String sigma_str = ut::Print(sigma);
	sigma_str.Print("%.2f", sigma);

	const ut::String shader_name = radius_str + "_" + sigma_str;

	Shader::Macros macros;
	Shader::MacroDefinition macro;

	macro.name = "GAUSSIAN_BLUR_RADIUS";
	macro.value = radius_str;
	macros.Add(ut::Move(macro));

	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                                ut::String("gaussian_blur_") + shader_name,
	                                                                "GaussianBlurPS",
	                                                                "gaussian_blur.hlsl",
	                                                                ut::Move(macros));
	return shader;
}

// Calculates gaussian blur weights for the desired radius and sigma.
ut::Array<float> GaussianBlur::CalculateWeights(ut::uint32 radius, float sigma)
{
	ut::Array<float> weights(radius * 2 + 1);

	const ut::int32 iradius = static_cast<ut::int32>(radius);
	float weight_sum = 0.0f;

	const ut::int32 kernel_size = static_cast<ut::int32>(weights.Count());
	for (ut::int32 i = -iradius; i <= iradius; ++i)
	{
		const float weight = ut::Exp(-((i * i) / (2.0f * sigma * sigma))) / (sigma * ut::Sqrt(2.0f * ut::Precision<float>::pi));
		weights[i + iradius] = weight;
		weight_sum += weight;
	}
	
	for (ut::int32 i = 0; i < kernel_size; ++i)
	{
		weights[i] /= weight_sum;
	}

	return weights;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//