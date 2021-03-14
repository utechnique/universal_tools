//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_fxaa.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// FXAA_PRESET shader macro value.
const ut::uint32 Fxaa::skDefaultPresetId = 5;

//----------------------------------------------------------------------------//
// View data constructor.
Fxaa::ViewData::ViewData(PipelineState fxaa_pipeline_state,
                         Buffer fxaa_uniform_buffer) : pipeline_state(ut::Move(fxaa_pipeline_state))
                                                     , uniform_buffer(ut::Move(fxaa_uniform_buffer))
{}

//----------------------------------------------------------------------------//
// Constructor.
Fxaa::Fxaa(Toolset& toolset) : tools(toolset)
                             , pixel_shader(LoadFxaaShader())
{}

// Creates fxaa (per-view) data.
//    @param postprocess_pass - render pass that will be used for fxaa.
//    @param width - width of the view in pixels.
//    @param height - height of the view in pixels.
//    @return - a new Fxaa::ViewData object or error if failed.
ut::Result<Fxaa::ViewData, ut::Error> Fxaa::CreateViewData(RenderPass& postprocess_pass,
                                                           ut::uint32 width,
                                                           ut::uint32 height)
{
	// pipeline
	PipelineState::Info info;
	info.stages[Shader::vertex] = tools.shaders.quad_vs;
	info.stages[Shader::pixel] = pixel_shader;
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(width),
	                            static_cast<float>(height),
	                            0.0f, 1.0f,
	                            width, height));
	info.input_assembly_state = tools.rc_mgr.fullscreen_quad->input_assembly;
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::never;
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	ut::Result<PipelineState, ut::Error> pipeline_state = tools.device.CreatePipelineState(ut::Move(info),
	                                                                                       postprocess_pass);
	if (!pipeline_state)
	{
		return ut::MakeError(pipeline_state.MoveAlt());
	}

	// create uniform buffer
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::uniform;
	buffer_info.usage = render::memory::gpu_read_cpu_write;
	buffer_info.size = sizeof(ViewData::FxaaUB);
	ut::Result<Buffer, ut::Error> uniform_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!uniform_buffer)
	{
		throw ut::Error(uniform_buffer.MoveAlt());
	}

	// create final data object
	ViewData data(pipeline_state.Move(), uniform_buffer.Move());

	// connect descriptors
	data.desc_set.Connect(pixel_shader);

	// success
	return data;
}

// Applies fxaa effect.
//    @param context - reference to the rendering context.
//    @param data - reference to the Fxaa::ViewData object containing
//                  fxaa-specific resources.
//    @param fb - reference to the framebuffer (with bound destination target).
//    @param pass - reference to the render pass with one color attachment
//                  and no depth.
//    @param source - reference to the source image.
void Fxaa::Apply(Context& context,
                 Fxaa::ViewData& data,
                 Framebuffer& fb,
                 RenderPass& pass,
                 Image& source)
{
	const Framebuffer::Info& fb_info = fb.GetInfo();

	// update uniform buffer
	ViewData::FxaaUB fxaa_ub;
	fxaa_ub.texel_size = ut::Vector<4>(1.0f / fb_info.width, 1.0f / fb_info.height, 0, 0);
	ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                                                    data.uniform_buffer,
	                                                                    &fxaa_ub);
	if (update_ub_error)
	{
		throw update_ub_error.Move();
	}

	// set shader resources
	data.desc_set.ub.BindUniformBuffer(data.uniform_buffer);
	data.desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);
	data.desc_set.tex2d.BindImage(source);

	// draw quad
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(pass, fb, render_area, ut::Color<4>(0), 1.0f);
	context.BindPipelineState(data.pipeline_state);
	context.BindDescriptorSet(data.desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->vertex_buffer, 0);
	context.Draw(6, 0);
	context.EndRenderPass();
}

// Returns compiled fxaa pixel shader.
Shader Fxaa::LoadFxaaShader()
{
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::pixel,
	                                                                "fxaa_ps",
	                                                                "PS",
	                                                                "fxaa.hlsl",
	                                                                GenShaderMacros(skDefaultPresetId));
	return shader.MoveOrThrow();
}

// Generates an array of macros for fxaa shader.
//    @param preset_id - quality preset (from 0 to 5).
//    @return - an array of macros.
Shader::Macros Fxaa::GenShaderMacros(ut::uint32 preset_id)
{
	Shader::Macros out;
	Shader::MacroDefinition preset;
	preset.name = "FXAA_PRESET";
	preset.value = ut::Print(preset_id);
	out.Add(ut::Move(preset));
	return out;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//