//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/policy/ve_render_view_policy.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Policy<View>::Policy(Toolset &toolset,
                     UnitSelector& unit_selector,
                     Policies& engine_policies) : tools(toolset)
                                                , selector(unit_selector)
                                                , policies(engine_policies)
                                                , geometry_pass(CreateGeometryPass(toolset.device).MoveOrThrow())
                                                , geometry_pass_shader(CreateGeometryPassShader(toolset.shader_loader))
{}

//----------------------------------------------------------------------------->
// Initializes provided view unit.
void Policy<View>::Initialize(View& view)
{
	ut::Array<View::FrameData> frames;
	for (ut::uint32 i = 0; i < tools.config.frames_in_flight; i++)
	{
		Target::Info info;
		info.type = Image::type_2D;
		info.format = skDepthFormat;
		info.usage = Target::Info::usage_depth;
		info.mip_count = 1;
		info.width = view.width;
		info.height = view.height;
		info.depth = 1;
		ut::Result<Target, ut::Error> depth = tools.device.CreateTarget(info);
		if (!depth)
		{
			throw ut::Error(depth.MoveAlt());
		}

		info.format = skGBufferFormat;
		info.usage = Target::Info::usage_color;
		ut::Result<Target, ut::Error> diffuse = tools.device.CreateTarget(info);
		if (!diffuse)
		{
			throw ut::Error(depth.MoveAlt());
		}

		ut::Result<Target, ut::Error> normal = tools.device.CreateTarget(info);
		if (!normal)
		{
			throw ut::Error(normal.MoveAlt());
		}

		ut::Result<Target, ut::Error> material = tools.device.CreateTarget(info);
		if (!material)
		{
			throw ut::Error(material.MoveAlt());
		}

		ut::Array< ut::Ref<Target> > color_targets;
		color_targets.Add(diffuse.Get());

		// create framebuffer
		ut::Result<Framebuffer, ut::Error> framebuffer = tools.device.CreateFramebuffer(geometry_pass,
			ut::Move(color_targets),
			depth.Get());
		if (!framebuffer)
		{
			throw ut::Error(framebuffer.MoveAlt());
		}

		View::GBuffer g_buffer(depth.Move(),
			diffuse.Move(),
			normal.Move(),
			material.Move(),
			framebuffer.Move());

		// create view uniform buffer
		Buffer::Info buffer_info;
		buffer_info.type = Buffer::uniform;
		buffer_info.usage = render::memory::gpu_read_cpu_write;
		buffer_info.size = sizeof(View::FrameData::ViewUB);
		ut::Result<Buffer, ut::Error> view_ub = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!view_ub)
		{
			throw ut::Error(view_ub.MoveAlt());
		}

		View::FrameData frame_data(ut::Move(g_buffer), view_ub.Move());
		frame_data.geometry_pass_desc_set.Connect(geometry_pass_shader);

		if (!frames.Add(ut::Move(frame_data)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	// pipeline state
	ut::Result<PipelineState, ut::Error> pipeline = CreateGeometryPassPipeline(view.width, view.height);
	if (!pipeline)
	{
		throw ut::Error(pipeline.MoveAlt());
	}

	View::GpuData gpu_data(ut::Move(frames), pipeline.Move());

	view.data = tools.rc_mgr.AddResource(ut::Move(gpu_data));

	view.data.Get();
}

//----------------------------------------------------------------------------->
// Renders all render units to all render views.
void Policy<View>::RenderEnvironment(Context& context)
{
	const ut::uint32 current_frame_id = tools.frame_mgr.GetCurrentFrameId();

	ut::Array< ut::Ref<View> >& views = selector.Get<View>();
	const size_t view_count = views.GetNum();
	for (size_t i = 0; i < view_count; i++)
	{
		View& view = views[i];

		// skip if view is inactive
		if (!view.is_active)
		{
			continue;
		}

		View::FrameData& frame = view.data->frames[current_frame_id];

		RcRef<Mesh>& mesh = tools.cube;

		// update view uniform buffer
		View::FrameData::ViewUB view_ub;
		view_ub.view_proj = view.view_matrix * view.proj_matrix;
		tools.rc_mgr.UpdateBuffer(context, frame.view_ub, &view_ub);

		// set uniforms
		frame.geometry_pass_desc_set.view_ub.BindUniformBuffer(frame.view_ub);

		ut::Rect<ut::uint32> render_area(0, 0, view.width, view.height);
		context.BeginRenderPass(geometry_pass, frame.g_buffer.framebuffer, render_area, ut::Color<4>(0.01f, 0.01f, 0.01f, 1.0f), 1.0f);
		context.BindPipelineState(view.data->geometry_pass_pipeline);
		context.BindDescriptorSet(frame.geometry_pass_desc_set);
		context.BindVertexBuffer(mesh->vertex_buffer, 0);

		if (mesh->index_buffer)
		{
			context.BindIndexBuffer(mesh->index_buffer.Get(), 0, mesh->index_type);
			context.DrawIndexed(mesh->face_count * Mesh::skPolygonVertices, 0, 0);
		}
		else
		{
			context.Draw(mesh->vertex_count, 0);
		}
		
		context.EndRenderPass();

		context.SetTargetState(frame.g_buffer.framebuffer, Target::Info::state_resource);
	}
}

//----------------------------------------------------------------------------->
// Creates a render pass for a g-buffer.
ut::Result<RenderPass, ut::Error> Policy<View>::CreateGeometryPass(Device& device)
{
	RenderTargetSlot depth_slot(skDepthFormat, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);
	RenderTargetSlot color_slot(skGBufferFormat, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, false);

	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);

	return device.CreateRenderPass(ut::Move(color_slots), depth_slot);
}

//----------------------------------------------------------------------------->
// Creates shaders for rendering geometry to a g-buffer.
BoundShader Policy<View>::CreateGeometryPassShader(ShaderLoader& shader_loader)
{
	ut::Result<Shader, ut::Error> vs = shader_loader.Load(Shader::vertex, "geometry_pass_vs", "VS", "geometry_pass.hlsl");
	ut::Result<Shader, ut::Error> ps = shader_loader.Load(Shader::pixel, "geometry_pass_ps", "PS", "geometry_pass.hlsl");
	return BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow());
}

//----------------------------------------------------------------------------->
// Creates pipeline for rendering geometry to a g-buffer.
ut::Result<PipelineState, ut::Error> Policy<View>::CreateGeometryPassPipeline(ut::uint32 width,
	ut::uint32 height)
{
	PipelineState::Info info;
	info.stages[Shader::vertex] = geometry_pass_shader.stages[Shader::vertex].Get();
	info.stages[Shader::pixel] = geometry_pass_shader.stages[Shader::pixel].Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
		static_cast<float>(width),
		static_cast<float>(height),
		0.0f, 1.0f,
		static_cast<ut::uint32>(width),
		static_cast<ut::uint32>(height)));
	info.input_assembly_state = tools.cube->input_assembly;
	info.depth_stencil_state.depth_test_enable = true;
	info.depth_stencil_state.depth_write_enable = true;
	info.depth_stencil_state.depth_compare_op = compare::less;
	info.rasterization_state.polygon_mode = RasterizationState::line;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(ut::Move(info), geometry_pass);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//