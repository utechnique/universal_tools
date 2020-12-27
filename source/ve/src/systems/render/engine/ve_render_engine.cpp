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
                                                                    , tools(render_device)
                                                                    , unit_mgr(tools)
                                                                    , profiler(tools)
{
	// set vertical synchronization for viewports
	SetVerticalSynchronization(tools.config.vsync);

	// load display shaders
	ut::Result<Shader, ut::Error> display_vs = tools.shader_loader.Load(Shader::vertex, "display_vs", "VS", "display.hlsl");
	ut::Result<Shader, ut::Error> display_ps = tools.shader_loader.Load(Shader::pixel, "display_ps", "PS", "display.hlsl");
	display_quad_shader = ut::MakeUnique<BoundShader>(display_vs.MoveOrThrow(), display_ps.MoveOrThrow());

	// 2d image
	ImageLoader::Info img_info_2d;
	img_info_2d.srgb = true;
	img_info_2d.high_quality_mips = false;
	ut::Result<Image, ut::Error> img2d_result = tools.img_loader.Load("maps/lazb.jpg", img_info_2d);
	img_2d = ut::MakeUnique<Image>(img2d_result.MoveOrThrow());

	// initialize per-frame data
	ut::Optional<ut::Error> frames_error = tools.frame_mgr.AllocateFrames(tools.config.frames_in_flight,
	                                                                      display_quad_shader.GetRef());
	if (frames_error)
	{
		throw frames_error.Move();
	}

	// save shader cache
	tools.shader_loader.SaveCache();
}

// Resets all previously added unit links.
void Engine::UnlinkUnits()
{
	unit_mgr.selector.Reset();
}

// Adds references to the provided units, these units will participate
// in the rendering process.
void Engine::LinkUnits(ut::Array< ut::UniquePtr<Unit> >& units)
{
	unit_mgr.selector.Select(units);
	unit_mgr.InitializeUnits();
}

// Renders the whole environment to the internal images and presents
// the result to user.
void Engine::ProcessNextFrame()
{
	// get current frame
	Frame& frame = tools.frame_mgr.GetCurrentFrame();

	// wait for the previous frame to finish
	device.WaitCmdBuffer(frame.cmd_buffer);

	// update resources (deletes unused resources, etc.)
	tools.rc_mgr.Update();

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
	tools.frame_mgr.SwapFrames();
}

// Function for recording all commands needed to draw current frame.
void Engine::RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportContainer> >& active_viewports)
{
	// get current frame
	Frame& frame = tools.frame_mgr.GetCurrentFrame();

	// render environment to view units
	Policy<View>& view_policy = unit_mgr.policies.Get<View>();
	view_policy.RenderEnvironment(context);

	// display rendered image to user
	for (size_t i = 0; i < active_viewports.GetNum(); i++)
	{
		ui::PlatformViewport& ui_viewport = active_viewports[i]->Get<ui::PlatformViewport&>();
		Display& display = active_viewports[i]->Get<Display>();
		RenderPass& rp = active_viewports[i]->Get<RenderPass>();
		PipelineState& pipeline_state = active_viewports[i]->Get<PipelineState>();
		ut::Array<Framebuffer>& framebuffers = active_viewports[i]->Get< ut::Array<Framebuffer> >();

		Framebuffer& framebuffer = framebuffers[display.GetCurrentBufferId()];
		const Framebuffer::Info& fb_info = framebuffer.GetInfo();
		ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);

		// update uniform buffer
		Frame::DisplayUB display_ub;
		display_ub.color = ut::Color<4>(1, 1, 1, 1);
		ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context,
		                                                                    frame.display_quad_ub,
		                                                                    &display_ub);
		if (update_ub_error)
		{
			throw update_ub_error.Move();
		}

		// draw quad
		frame.quad_desc_set.ub.BindUniformBuffer(frame.display_quad_ub);
		frame.quad_desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);

		if (ui_viewport.GetId() == 0)
		{
			ut::Array< ut::Ref<View> > views = unit_mgr.selector.Get<View>();

			if (!views.IsEmpty())
			{
				frame.quad_desc_set.tex2d.BindImage(views.GetFirst()->data->frames[tools.frame_mgr.GetCurrentFrameId()].g_buffer.diffuse.GetImage());
			}
			else
			{
				frame.quad_desc_set.tex2d.BindImage(img_2d.GetRef());
			}
		}
		else
		{
			frame.quad_desc_set.tex2d.BindImage(img_2d.GetRef());
		}


		context.BeginRenderPass(rp, framebuffer, render_area, frame.clear_color);
		context.BindPipelineState(pipeline_state);
		context.BindDescriptorSet(frame.quad_desc_set);
		context.BindVertexBuffer(tools.fullscreen_quad->vertex_buffer, 0);
		context.Draw(6, 0);

		// draw profiler info
		if (ui_viewport.GetId() == 0)
		{
			profiler.DrawInfo(context, frame, display.GetWidth(), display.GetHeight());
		}

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
		tools.frame_mgr.WaitCmdBuffers();

		// viewport tasks can now be executed safely
		ExecuteViewportTasks();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
