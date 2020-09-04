//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_engine.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Engine::Engine(Device& render_device, ViewportManager viewport_mgr) : ViewportManager(ut::Move(viewport_mgr))
                                                                    , device(render_device)
                                                                    , current_frame_id(0)
{
	// load configuration file
	ut::Optional<ut::Error> load_cfg_error = config.Load();
	if (load_cfg_error)
	{
		const ut::error::Code error_code = load_cfg_error->GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "Render config file is absent. Using default configuration..." << ut::cret;
			config.Save();
		}
		else
		{
			ut::log << "Fatal error while loading render config file." << ut::cret;
			throw load_cfg_error.Move();
		}
	}

	// load shader cache
	shader_cache.Load();

	// set vertical synchronization for viewports
	SetVerticalSynchronization(config.vsync);

	// initialize color to clear display with
	backbuffer_clear_values.Add(ut::Color<4>(0, 0, 0, 0));







	// load display shaders
	ut::Result<Shader, ut::Error> display_vs = LoadShader(Shader::vertex, "display_vs", "VS", "display.hlsl");
	ut::Result<Shader, ut::Error> display_ps = LoadShader(Shader::pixel, "display_ps", "PS", "display.hlsl");
	display_shader = ut::MakeUnique<BoundShader>(display_vs.MoveOrThrow(), display_ps.MoveOrThrow());


	struct QuadVertex
	{
		ut::Vector<3> p;
		ut::Vector<2> t;
	};

	// create screen space quad
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::vertex;
	buffer_info.usage = render::memory::immutable;
	buffer_info.size = sizeof(QuadVertex) * 4;
	buffer_info.stride = sizeof(QuadVertex);
	buffer_info.data.Resize(buffer_info.size);

	QuadVertex* vertices = reinterpret_cast<QuadVertex*>(buffer_info.data.GetAddress());
	vertices[0].p = ut::Vector<3>(-1, -1, 0);
	vertices[1].p = ut::Vector<3>(-1,  1, 0);
	vertices[2].p = ut::Vector<3>( 1, -1, 0);
	vertices[3].p = ut::Vector<3>( 1,  1, 0);

	ut::Result<Buffer, ut::Error> buffer_result = device.CreateBuffer(ut::Move(buffer_info));
	screen_space_quad = ut::MakeUnique<Buffer>(buffer_result.MoveOrThrow());


	// initialize per-frame data
	for (size_t i = 0; i < config.frames_in_flight; i++)
	{
		ut::Result<Frame, ut::Error> frame = CreateFrame();
		if (!frames.Add(frame.MoveOrThrow()))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	// save shader cache
	shader_cache.Save();
}

// Renders the whole environment to the internal images and presents
// the result to user.
void Engine::ProcessNextFrame()
{
	// wrap frame id
	if (current_frame_id >= frames.GetNum())
	{
		current_frame_id = 0;
	}

	// get current frame
	Frame& frame = frames[current_frame_id];

	// wait for the previous frame to finish
	device.WaitCmdBuffer(frame.cmd_buffer);

	// execute viewport tasks (resize, close, etc.)
	ProcessViewportEvents();

	// get active viewports
	ut::Array< ut::Ref<ViewportContainer> > active_viewports;
	for (size_t i = 0; i < viewports.GetNum(); i++)
	{
		ui::PlatformViewport& viewport = viewports[i].Get<ui::PlatformViewport&>();
		if (viewport.IsActive())
		{
			active_viewports.Add(viewports[i]);
		}
	}

	// generate an array of active displays
	ut::Array< ut::Ref<Display> > display_array;
	for (size_t i = 0; i < active_viewports.GetNum(); i++)
	{
		display_array.Add(active_viewports[i]->Get<Display>());
		device.AcquireNextDisplayBuffer(display_array.GetLast());
	}

	// record all commands for the current frame
	device.Record(frame.cmd_buffer, [&](Context& context) { RecordFrameCommands(context, active_viewports); });

	// submit commands and enqueue display presentation
	device.Submit(frame.cmd_buffer, display_array);

	// go to the next frame
	current_frame_id++;
}

// Function for recording all commands needed to draw current frame.
void Engine::RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportContainer> >& active_viewports)
{
	// get current frame
	Frame& frame = frames[current_frame_id];

	for (size_t i = 0; i < active_viewports.GetNum(); i++)
	{
		Display& display = active_viewports[i]->Get<Display>();
		RenderPass& rp = active_viewports[i]->Get<RenderPass>();
		PipelineState& pipeline_state = active_viewports[i]->Get<PipelineState>();

		static int it = 0;
		it++;
		bool swap_col = it % 100 < 50;
		if (swap_col)
		{
			switch (i)
			{
			case 0: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 0, 1); break;
			case 1: backbuffer_clear_values[0] = ut::Color<4>(0, 1, 0, 1); break;
			case 2: backbuffer_clear_values[0] = ut::Color<4>(0, 0, 1, 1); break;
			case 3: backbuffer_clear_values[0] = ut::Color<4>(1, 1, 0, 1); break;
			}
		}
		else
		{
			switch (i)
			{
			case 0: backbuffer_clear_values[0] = ut::Color<4>(0, 0, 1, 1); break;
			case 1: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 1, 1); break;
			case 2: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 0, 1); break;
			case 3: backbuffer_clear_values[0] = ut::Color<4>(0, 1, 1, 1); break;
			}
		}

		ut::Array<Framebuffer>& framebuffers = active_viewports[i]->Get< ut::Array<Framebuffer> >();

		Framebuffer& framebuffer = framebuffers[display.GetCurrentBufferId()];
		const FramebufferInfo& fb_info = framebuffer.GetInfo();
		ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);

		// update uniform buffer
		ut::Color<4> display_color(1, 0, swap_col ? 1 : 0, 1);
		UpdateBuffer(context, frame.display_ub, &display_color);

		// draw quad
		frame.quad_desc_set.ub.BindUniformBuffer(frame.display_ub);

		context.BeginRenderPass(rp, framebuffer, render_area, backbuffer_clear_values);

		context.BindPipelineState(pipeline_state);
		context.BindDescriptorSet(frame.quad_desc_set);
		context.BindVertexBuffer(screen_space_quad.GetRef(), 0);
		context.Draw(4, 0);

		context.EndRenderPass();
	}
}

// Executes viewport tasks (resize, close, etc.) in a safe manner.
void Engine::ProcessViewportEvents()
{
	// synchronize viewport events in this point
	SyncViewportEvents();

	// execute viewport tasks
	if (HasPendingViewportTasks())
	{
		// all frames in flight must be finished to prevent
		// damaging resources bound to the rendering pipeline
		for (size_t i = 0; i < frames.GetNum(); i++)
		{
			device.WaitCmdBuffer(frames[i].cmd_buffer);
		}

		// viewport tasks can now be executed safely
		ExecuteViewportTasks();
	}
}

// Compiles a shader from file. Returns cached version if source didn't
// change or compiles it from the scratch otherwise.
//    @param stage - type of the shader (vertex/pixel/geometry etc.).
//    @param shader_name - string with the name of this
//                         particular shader build.
//    @param entry_point - string with a name of entry point.
//    @param filename - const string with the name of shader file, can be
//                      relative to ve::directories::skRc directory.
//    @param macros - preprocessor macros to build shader with.
//    @return - Shader::Info object or ut::Error if failed.
ut::Result<Shader, ut::Error> Engine::LoadShader(Shader::Stage stage,
                                                 ut::String shader_name,
                                                 ut::String entry_point,
                                                 const ut::String& filename,
                                                 Shader::Macros macros)
{
	ut::Result<Shader::Info, ut::Error> result = shader_cache.CompileFromFile(stage,
	                                                                          ut::Move(shader_name),
	                                                                          ut::Move(entry_point),
	                                                                          filename,
	                                                                          ut::Move(macros));
	if (!result)
	{
		return ut::MakeError(result.MoveAlt());
	}
	return device.CreateShader(result.Move());
}

// Updates buffer contents with provided data. Can be used as a convenient
// wrapper around MapBuffer + UnmapBuffer functions. Note that buffer must be
// created with  ve::render::Buffer::gpu_cpu flag to be compatible with this
// function.
//    @param context - reference to the ve::render::Context object to perform
//                     perform update operation.
//    @param buffer - reference to the ve::render::Buffer object to be updated.
//    @param data - reference to the ve::render::Buffer object to be updated.
ut::Optional<ut::Error> Engine::UpdateBuffer(Context& context, Buffer& buffer, const void* data)
{
	ut::Result<void*, ut::Error> map_result = context.MapBuffer(buffer, ut::access_write);
	if (!map_result)
	{
		return map_result.MoveAlt();
	}

	ut::memory::Copy(map_result.Get(), data, buffer.GetInfo().size);

	context.UnmapBuffer(buffer);

	return ut::Optional<ut::Error>();
}

// Creates a set of frame data.
//    @return - ve::render::Frame object or ut::Error if failed.
ut::Result<Frame, ut::Error> Engine::CreateFrame()
{
	// initialize command buffer info
	CmdBuffer::Info cmd_buffer_info;
	cmd_buffer_info.usage = CmdBuffer::usage_dynamic;

	// create command buffer
	ut::Result<CmdBuffer, ut::Error> cmd_buffer = device.CreateCmdBuffer(cmd_buffer_info);
	if (!cmd_buffer)
	{
		return ut::MakeError(cmd_buffer.MoveAlt());
	}

	// create display uniform buffer
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::uniform;
	buffer_info.usage = render::memory::gpu_cpu;
	buffer_info.size = sizeof(ut::Color<4>);
	buffer_info.data.Resize(buffer_info.size);
	ut::Color<4>* color = reinterpret_cast<ut::Color<4>*>(buffer_info.data.GetAddress());
	*color = ut::Color<4>(1, 0, 1, 0);
	ut::Result<Buffer, ut::Error> display_ub = device.CreateBuffer(ut::Move(buffer_info));
	if (!display_ub)
	{
		return ut::MakeError(display_ub.MoveAlt());
	}
	
	// create frame
	Frame frame(cmd_buffer.Move(), display_ub.Move());

	// connect descriptors
	frame.quad_desc_set.Connect(display_shader.GetRef());

	// success
	return frame;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//