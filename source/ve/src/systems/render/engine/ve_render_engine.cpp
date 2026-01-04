//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_engine.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Engine::Engine(Device& render_device,
               ViewportManager viewport_mgr) : ViewportManager(ut::Move(viewport_mgr))
                                             , device(render_device)
                                             , tools(render_device)
                                             , unit_mgr(tools)
{
	// set vertical synchronization and the number of swap buffers
	SetSwapBufferCount(tools.config.swapchain_buffer_count);
	SetVerticalSynchronization(tools.config.vsync);

	// create pipeline state for displaying images to user
	const size_t quad_ps_id = static_cast<size_t>(QuadRenderer::ColorSpaceCvt::none);
	ut::Optional<ut::Error> frames_error = tools.frame_mgr.AllocateFrames(tools.config.frames_in_flight,
	                                                                      tools.quad.ps[QuadRenderer::ShaderGrid::GetId(quad_ps_id)]);
	if (frames_error)
	{
		throw frames_error.Move();
	}

	// save shader cache
	tools.shader_loader.SaveCache();
}

// Includes provided units to the rendering scene.
void Engine::RegisterEntity(Entity::Id id,
                            ut::Array< ut::UniquePtr<Unit> >& units)
{
	unit_mgr.selector.Select(id, units);

	// mesh instance units must be registered separately
	Policy<MeshInstance>& mesh_instance_policy = unit_mgr.policies.Get<MeshInstance>();
	mesh_instance_policy.batcher.Register(id, units);
}

// Excludes units of the specified entity from the rendering scene.
void Engine::UnregisterEntity(Entity::Id id)
{
	unit_mgr.selector.Remove(id);

	// mesh instance units must be unregistered separately
	Policy<MeshInstance>& mesh_instance_policy = unit_mgr.policies.Get<MeshInstance>();
	mesh_instance_policy.batcher.Unregister(id);
}

// Initializes a unit with the correct policy.
void Engine::InitializeUnit(Unit& unit)
{
	unit_mgr.InitializeUnit(unit);
}

// Renders the whole environment to the internal images and presents
// the result to user.
//    @param time_step_ms - time step in milliseconds.
void Engine::ProcessNextFrame(System::Time time_step_ms)
{
	// get current frame
	Frame& frame = UpdateCurrentFrameInfo();

	// update profiler
	tools.profiler.ResetCounterAndUpdateFrameId();

	// wait for the previous frame to finish
	{
		ut::Optional<Profiler::ScopeCounter> wait_gpu_cmd_scope_counter =
			tools.profiler.CreateScopeCounter(Profiler::Stat::wait_gpu);

		device.WaitCmdBuffer(frame.cmd_buffer);
	}

	// update resources (deletes unused resources, etc.)
	tools.rc_mgr.Update();

	// execute viewport tasks (resize, close, etc.)
	ProcessViewportEvents();

	// get active viewports
	ut::Array< ut::Ref<ViewportManager::Proxy> > active_viewports;
	for (size_t i = 0; i < viewports.Count(); i++)
	{
		ui::PlatformViewport& viewport_ui_widget = viewports[i].ui_widget;
		const ui::Viewport::Mode mode = viewport_ui_widget.GetMode();
		if (mode.is_active)
		{
			active_viewports.Add(viewports[i]);
		}
	}

	// generate an array of active displays
	ut::Array< ut::Ref<Display> > display_array;
	{
		for (size_t i = 0; i < active_viewports.Count(); i++)
		{
			display_array.Add(active_viewports[i]->display);
			device.AcquireNextDisplayBuffer(display_array.GetLast());
		}
	}

	// record all commands for the current frame
	device.Record(frame.cmd_buffer, [&](Context& context) { RecordFrameCommands(context,
	                                                                            active_viewports,
	                                                                            time_step_ms); });

	// submit commands and enqueue display presentation
	ut::Optional<Profiler::ScopeCounter> submit_scope_counter =
		tools.profiler.CreateScopeCounter(Profiler::Stat::submit);
	device.Submit(frame.cmd_buffer, display_array);

	// go to the next frame
	tools.frame_mgr.SwapFrames();
}

// Returns a reference to the rendering thread pool, 
// use it to parallelize cpu work.
ut::ThreadPool<void>& Engine::GetThreadPool()
{
	return tools.pool;
}

// Returns a reference to the profiler object,
// use it to measure performance.
Profiler& Engine::GetProfiler()
{
	return tools.profiler;
}

// Updates the information about the current frame and returns the
// reference to this frmae.
Frame& Engine::UpdateCurrentFrameInfo()
{
	Frame& frame = tools.frame_mgr.GetCurrentFrame();

	// reset temp flags
	frame.info.needs_entity_id_buffer_update = false;

	// check if at least one view unit needs hitmask update,
	// and if so - enable needs_entity_id_buffer_update flag
	const ut::Array< ut::Ref<View> >& views = unit_mgr.selector.Get<View>();
	const size_t view_count = views.Count();
	for (size_t i = 0; i < view_count; i++)
	{
		const View& view = views[i];
		if (view.draw_hitmask)
		{
			frame.info.needs_entity_id_buffer_update = true;
			break;
		}
	}

	return frame;
}

// Function for recording all commands needed to draw current frame.
void Engine::RecordFrameCommands(Context& context,
                                 ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports,
                                 System::Time time_step_ms)
{
	// collect the timestamps and reset queries from the previous cycle
	tools.profiler.CollectQueriesAndSaveResults(context);

	ut::Optional<Profiler::ScopeCounter> scope_counter =
		tools.profiler.CreateScopeCounter(Profiler::Stat::frame, context);

	// generate global transform buffer for all mesh instance units
	Policy<MeshInstance>& mesh_instance_policy = unit_mgr.policies.Get<MeshInstance>();
	mesh_instance_policy.batcher.UpdateBuffers(context);

	// render environment to view units
	Policy<View>& view_policy = unit_mgr.policies.Get<View>();
	view_policy.RenderEnvironment(context, time_step_ms);

	// display view units to user
	DisplayToUser(context, active_viewports);
}

// Renders view units to ui viewports.
void Engine::DisplayToUser(Context& context, ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports)
{
	ut::Array< ut::Ref<View> >& views = unit_mgr.selector.Get<View>();
	const size_t view_count = views.Count();
	
	for (size_t i = 0; i < active_viewports.Count(); i++)
	{
		// find appropriate view unit and extract an image
		ut::Optional<Image&> image;
		const ui::PlatformViewport& ui_viewport_widget = active_viewports[i]->ui_widget;
		for (size_t unit_index = 0; unit_index < view_count; unit_index++)
		{
			View& view = views[unit_index];
			if (view.viewport_id == ui_viewport_widget.GetId())
			{
				image = view.data->frames[tools.frame_mgr.GetCurrentFrameId()].final_img;
				break;
			}
		}

		// display to user
		const bool display_profiler_stat = tools.config.show_fps ||
		                                   tools.config.show_render_stat;
		DisplayImage(context,
		             image ? image.Get() : tools.rc_mgr.img_black.Get(),
		             active_viewports[i],
		             display_profiler_stat);
	}
}

// Displays provided image to the provided ui viewport.
void Engine::DisplayImage(Context& context,
                          Image& image,
                          ViewportManager::Proxy& viewport,
                          bool display_profiler)
{
	Frame& frame = tools.frame_mgr.GetCurrentFrame();

	// get framebuffer
	const ut::uint32 display_buffer_id = viewport.display.GetCurrentBufferId();
	Framebuffer& framebuffer = viewport.framebuffers[display_buffer_id];
	const Framebuffer::Info& fb_info = framebuffer.GetInfo();

	// update uniform buffer
	Frame::DisplayUB display_ub;
	display_ub.color = ut::Color<4>(1);
	ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                                                    frame.display_quad_ub,
	                                                                    &display_ub);
	if (update_ub_error)
	{
		throw update_ub_error.Move();
	}

	// calculate backbuffer pipeline indices
	const size_t pipeline_alpha_blend_id =
		ViewportManager::Proxy::PipelineGrid::GetId(static_cast<size_t>(ViewportManager::Proxy::ColorSpaceCvt::none),
		                                            static_cast<size_t>(ViewportManager::Proxy::AlphaMode::blend));
	const size_t pipeline_srgb2rgb_no_alpha_id =
		ViewportManager::Proxy::PipelineGrid::GetId(static_cast<size_t>(ViewportManager::Proxy::ColorSpaceCvt::srgb2rgb),
		                                            static_cast<size_t>(ViewportManager::Proxy::AlphaMode::none));

	// choose pipeline state according to the color space conversion type
	PipelineState& pipeline_state = viewport.pipeline[pipeline_srgb2rgb_no_alpha_id];

	// set shader resources
	frame.quad_desc_set.ub.BindUniformBuffer(frame.display_quad_ub);
	frame.quad_desc_set.sampler.BindSampler(tools.sampler_cache.point_clamp);
	frame.quad_desc_set.tex2d.BindImage(image);

	// draw quad
	const ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(viewport.quad_pass, framebuffer, render_area, frame.clear_color);
	context.BindPipelineState(pipeline_state);
	context.BindDescriptorSet(frame.quad_desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->subsets.GetFirst().vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);

	// draw profiler info
	if (display_profiler)
	{
		context.BindPipelineState(viewport.pipeline[pipeline_alpha_blend_id]);
		tools.profiler.DrawStats(context,
		                         frame,
		                         viewport.ui_widget.GetId(),
		                         fb_info.width,
		                         fb_info.height);
	}

	context.EndRenderPass();
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
		tools.frame_mgr.WaitCmdBuffers();

		// viewport tasks can now be executed safely
		ExecuteViewportTasks();
	}
}

// Creates a new display and all associated render resources for the
// provided viewport.
//    @param viewport - reference to the viewport.
//    @return - container with all render resources, or error if failed.
ut::Result<ViewportManager::Proxy, ut::Error> Engine::CreateDisplay(ui::PlatformViewport& viewport)
{
	// create display for the viewport in the render thread
	ut::Result<Display, ut::Error> display_result = device.CreateDisplay(viewport,
	                                                                     GetSwapBufferCount(),
	                                                                     IsVerticalSynchronizationEnabled());
	if (!display_result)
	{
		return ut::MakeError(display_result.MoveAlt());
	}

	// check if display has at least one target
	const ut::uint32 buffer_count = display_result.Get().GetBufferCount();
	if (buffer_count == 0)
	{
		return ut::MakeError(ut::Error(ut::error::empty, "Render: display has empty target list."));
	}

	// initialize render pass info
	pixel::Format format = display_result.Get().GetTarget(0).GetImage().GetInfo().format;
	RenderTargetSlot color_slot(format,
	                            RenderTargetSlot::LoadOperation::clear,
	                            RenderTargetSlot::StoreOperation::save,
	                            true);
	ut::Array<RenderTargetSlot> slots;
	slots.Add(color_slot);

	// create render pass for this display
	ut::Result<RenderPass, ut::Error> rp_result = device.CreateRenderPass(ut::Move(slots));
	if (!rp_result)
	{
		return ut::MakeError(rp_result.MoveAlt());
	}

	// create framebuffer for every target
	ut::Array<Framebuffer> framebuffers;
	for (ut::uint32 i = 0; i < buffer_count; i++)
	{
		// initialize framebuffer info
		ut::Array<Framebuffer::Attachment> color_targets;
		color_targets.Add(display_result.Get().GetTarget(i));

		// create framebuffer
		ut::Result<Framebuffer, ut::Error> fb_result = device.CreateFramebuffer(rp_result.Get(),
		                                                                        ut::Move(color_targets));
		if (!fb_result)
		{
			return ut::MakeError(fb_result.MoveAlt());
		}

		// add framebuffer to the array
		if (!framebuffers.Add(fb_result.Move()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// create pipeline state for displaying images to user
	ut::Array<PipelineState> pipeline;
	constexpr size_t permutation_count = ViewportManager::Proxy::PipelineGrid::size;
	for (size_t i = 0; i < permutation_count; i++)
	{
		const size_t color_space_cvt = Proxy::PipelineGrid::GetCoordinate<Proxy::color_space_cvt_column>(i);
		const Proxy::AlphaMode alpha_mode = static_cast<Proxy::AlphaMode>(
			Proxy::PipelineGrid::GetCoordinate<Proxy::alpha_mode_column>(i));

		Shader& vertex_shader = tools.quad.vs;
		Shader& pixel_shader = tools.quad.ps[QuadRenderer::ShaderGrid::GetId(color_space_cvt)];

		PipelineState::Info info;
		info.SetShader(Shader::Stage::vertex, vertex_shader);
		info.SetShader(Shader::Stage::pixel, pixel_shader);
		info.input_assembly_state = QuadRenderer::CreateInputAssemblyState();
		info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
		info.rasterization_state.cull_mode = RasterizationState::CullMode::off;

		if (alpha_mode == Proxy::AlphaMode::blend)
		{
			info.blend_state.attachments.Add(BlendState::CreateAlphaBlending());
		}
		else
		{
			info.blend_state.attachments.Add(BlendState::CreateNoBlending());
		}

		ut::Result<PipelineState,
		           ut::Error> pipeline_result = device.CreatePipelineState(info,
		                                                                   rp_result.Get());
		if (!pipeline_result)
		{
			return ut::MakeError(pipeline_result.MoveAlt());
		}

		if (!pipeline.Add(pipeline_result.Move()))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// success
	return Proxy(viewport,
	             display_result.Move(),
	             rp_result.Move(),
	             ut::Move(pipeline),
	             ut::Move(framebuffers));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
