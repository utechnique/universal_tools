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

	// set display shaders
	display_quad_vs = tools.shaders.quad_vs;
	display_quad_ps = tools.shaders.img_quad_ps;
	display_quad_rgb2srgb_ps = tools.shaders.img_quad_rgb2srgb_ps;
	display_quad_srgb2rgb_ps = tools.shaders.img_quad_srgb2rgb_ps;

	// 2d image
	ImageLoader::Info img_info_2d;
	img_info_2d.srgb = true;
	img_info_2d.high_quality_mips = false;
	ut::Result<Image, ut::Error> img2d_result = tools.img_loader.Load("maps/lazb.jpg", img_info_2d);
	img_2d = ut::MakeUnique<Image>(img2d_result.MoveOrThrow());

	// initialize per-frame data
	ut::Optional<ut::Error> frames_error = tools.frame_mgr.AllocateFrames(tools.config.frames_in_flight,
	                                                                      display_quad_ps.Get());
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
	ut::Array< ut::Ref<ViewportManager::Proxy> > active_viewports;
	for (size_t i = 0; i < viewports.GetNum(); i++)
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
	for (size_t i = 0; i < active_viewports.GetNum(); i++)
	{
		display_array.Add(active_viewports[i]->display);
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
void Engine::RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports)
{
	// render environment to view units
	Policy<View>& view_policy = unit_mgr.policies.Get<View>();
	view_policy.RenderEnvironment(context);

	// display view units to user
	DisplayToUser(context, active_viewports);
}

// Renders view units to ui viewports.
void Engine::DisplayToUser(Context& context, ut::Array< ut::Ref<ViewportManager::Proxy> >& active_viewports)
{
	ut::Array< ut::Ref<View> >& views = unit_mgr.selector.Get<View>();
	const size_t view_count = views.GetNum();
	
	for (size_t i = 0; i < active_viewports.GetNum(); i++)
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
		DisplayImage(context,
		             image ? image.Get() : tools.img_black,
		             active_viewports[i],
		             ui_viewport_widget.GetId() == 0);
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

	// check if rgb->srgb or srgb->rgb conversions are needed
	const bool src_is_srgb = pixel::IsSrgb(image.GetInfo().format);
	const bool dst_is_srgb = pixel::IsSrgb(viewport.display.GetTarget(display_buffer_id).GetInfo().format);
	const bool needs_rgb2srgb = dst_is_srgb && !src_is_srgb;
	const bool needs_srgb2rgb = !dst_is_srgb && src_is_srgb;

	// choose pipeline state according to the color space conversion type
	PipelineState& pipeline_state = needs_rgb2srgb ? viewport.pipeline_rgb2srgb :
		                            needs_srgb2rgb ? viewport.pipeline_srgb2rgb :
	                                viewport.pipeline_state;

	// set shader resources
	frame.quad_desc_set.ub.BindUniformBuffer(frame.display_quad_ub);
	frame.quad_desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);
	frame.quad_desc_set.tex2d.BindImage(image);

	// draw quad
	const ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(viewport.quad_pass, framebuffer, render_area, frame.clear_color);
	context.BindPipelineState(pipeline_state);
	context.BindDescriptorSet(frame.quad_desc_set);
	context.BindVertexBuffer(tools.fullscreen_quad->vertex_buffer, 0);
	context.Draw(6, 0);

	// draw profiler info
	if (display_profiler)
	{
		profiler.DrawInfo(context, frame, fb_info.width, fb_info.height);
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

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
