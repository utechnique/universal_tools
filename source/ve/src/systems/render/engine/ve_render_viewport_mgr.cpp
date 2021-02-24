//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_viewport_mgr.h"
#include "systems/render/engine/ve_render_cfg.h"
#include "systems/render/engine/ve_render_frame.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor. Render thread is needed to operate with
// displays that are associated with viewports.
ViewportManager::ViewportManager(ut::SharedPtr<Device::Thread> rt) : render_thread(ut::Move(rt))
                                                                   , vertical_synchronization(true)
{
	UT_ASSERT(render_thread);
}

// Destructor.
ViewportManager::~ViewportManager()
{
	// reset viewport signals, otherwise viewports will signal
	// to the non-existent viewport manager, causing access violation
	for (ut::uint32 i = 0; i < viewports.GetNum(); i++)
	{
		viewports[i].ui_widget.ResetSignals();
	}

	// close viewports manually
	for (ut::uint32 i = 0; i < viewports.GetNum(); i++)
	{
		CloseViewport(i);
	}
}

// Move constructor.
ViewportManager::ViewportManager(ViewportManager&&) = default;

// Initializes internal array of viewport containers.
//    @param frontend - reference to ui::Frontend object.
void ViewportManager::OpenViewports(ui::Frontend& frontend)
{
	ut::Array< ut::Ref<ui::Viewport> >::Iterator start = frontend.BeginViewports();
	ut::Array< ut::Ref<ui::Viewport> >::Iterator end = frontend.EndViewports();

	for (ut::Array< ut::Ref<ui::Viewport> >::Iterator iterator = start; iterator != end; iterator++)
	{
		// convert iterator to the reference to desktop viewport
		ut::Ref<ui::Viewport>& viewport_ptr = *iterator;
		ui::PlatformViewport& viewport = static_cast<ui::PlatformViewport&>(viewport_ptr.Get());

		// create render resources for the viewport
		ut::Result<Proxy, ut::Error> result = ut::MakeError(ut::error::empty);
		render_thread->Enqueue([&](Device& device) { result = CreateDisplay(device, viewport); });
		if (!result)
		{
			throw ut::Error(result.MoveAlt());
		}

		// newly created display is added to the map and can be
		// used to display rendered images to user
		if (!viewports.Add(result.Move()))
		{
			throw ut::Error(ut::error::out_of_memory);
		}

		// connect signals
		viewport.ConnectClose([&](ui::Viewport::Id id) { EnqueueClose(id); });
		viewport.ConnectResize([&](ui::Viewport::Id id, ut::uint32 w, ut::uint32 h){ EnqueueResize(id, w, h); });
	}
}

// Synchronizes all viewport events.
void ViewportManager::SyncViewportEvents()
{
	sync_point.Synchronize();
}

// Executes pending viewport tasks (resize, close, etc.)
void ViewportManager::ExecuteViewportTasks()
{
	const size_t task_count = viewport_tasks.GetNum();
	for (size_t i = 0; i < task_count; i++)
	{
		viewport_tasks[i]->Execute();
	}
	viewport_tasks.Empty();
}

// Returns 'true' if at least one viewport task is waiting to be executed.
bool ViewportManager::HasPendingViewportTasks()
{
	return !viewport_tasks.IsEmpty();
}

// Enables or disables vertical synchronization.
void ViewportManager::SetVerticalSynchronization(bool status)
{
	vertical_synchronization = status;
}

// Creates a new display and all associated render resources for the
// provided viewport.
//    @param device - reference to the render device.
//    @param viewport - reference to the viewport.
//    @return - container with all render resources, or error if failed.
ut::Result<ViewportManager::Proxy, ut::Error> ViewportManager::CreateDisplay(Device& device,
                                                                             ui::PlatformViewport& viewport)
{
	// shaders must be initialized at this point
	UT_ASSERT(display_quad_vs);
	UT_ASSERT(display_quad_ps);
	UT_ASSERT(display_quad_rgb2srgb_ps);
	UT_ASSERT(display_quad_srgb2rgb_ps);

	// create display for the viewport in the render thread
	ut::Result<Display, ut::Error> display_result = device.CreateDisplay(viewport, vertical_synchronization);
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
	RenderTargetSlot color_slot(format, RenderTargetSlot::load_clear, RenderTargetSlot::store_save, true);
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
		ut::Array< ut::Ref<Target> > color_targets;
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
	PipelineState::Info info;
	info.stages[Shader::vertex] = display_quad_vs.Get();
	info.stages[Shader::pixel] = display_quad_ps.Get();
	info.viewports.Add(Viewport(0.0f, 0.0f,
	                            static_cast<float>(viewport.w()),
	                            static_cast<float>(viewport.h()),
	                            0.0f, 1.0f,
	                            static_cast<ut::uint32>(viewport.w()),
	                            static_cast<ut::uint32>(viewport.h())));
	info.input_assembly_state = Frame::CreateInputAssemblyState();
	info.rasterization_state.polygon_mode = RasterizationState::fill;
	info.rasterization_state.cull_mode = RasterizationState::no_culling;
	info.blend_state.attachments.Add(BlendState::CreateAlphaBlending());
	ut::Result<PipelineState, ut::Error> pipeline = device.CreatePipelineState(info, rp_result.Get());
	if (!pipeline)
	{
		return ut::MakeError(pipeline.MoveAlt());
	}

	// pipeline with rgb->srgb conversion
	info.stages[Shader::pixel] = display_quad_rgb2srgb_ps.Get();
	ut::Result<PipelineState, ut::Error> rgb2srgb_pipeline = device.CreatePipelineState(info, rp_result.Get());
	if (!rgb2srgb_pipeline)
	{
		return ut::MakeError(rgb2srgb_pipeline.MoveAlt());
	}

	// pipeline with srgb->rgb conversion
	info.stages[Shader::pixel] = display_quad_srgb2rgb_ps.Get();
	ut::Result<PipelineState, ut::Error> srgb2rgb_pipeline = device.CreatePipelineState(info, rp_result.Get());
	if (!srgb2rgb_pipeline)
	{
		return ut::MakeError(srgb2rgb_pipeline.MoveAlt());
	}

	// success
	return Proxy(viewport,
	             display_result.Move(),
	             rp_result.Move(),
	             pipeline.Move(),
	             rgb2srgb_pipeline.Move(),
	             srgb2rgb_pipeline.Move(),
	             ut::Move(framebuffers));
}

// Resizes a display associated with provided viewport.
//    @param id - id of the viewport whose render display must be resized.
//    @param w - new width of render display in pixels.
//    @param h - new height of render display in pixels.
void ViewportManager::ResizeViewport(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h)
{
	render_thread->Enqueue([&](Device& device) {
		ut::Optional<size_t> find_result = FindViewport(id);
		if (!find_result)
		{
			return;
		}

		// get viewport reference
		ui::PlatformViewport& ui_widget = viewports[find_result.Get()].ui_widget;

		// delete current container
		viewports.Remove(find_result.Get());

		// re-create render resources
		ut::Result<Proxy, ut::Error> vp_container_result = CreateDisplay(device, ui_widget);
		if (!vp_container_result)
		{
			throw ut::Error(vp_container_result.MoveAlt());
		}

		// add new container to the map
		if (!viewports.Add(vp_container_result.Move()))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	});
}

// Removes viewport from internal map and destroys render resources.
//    @param id - id of the viewport to be closed.
void ViewportManager::CloseViewport(ui::Viewport::Id id)
{
	render_thread->Enqueue([&](Device& device)
	{
		device.WaitIdle();
		ut::Optional<size_t> find_result = FindViewport(id);
		if (find_result)
		{
			viewports.Remove(find_result.Get());
		}
	});
}

// Enqueues a task and waits for completion.
void ViewportManager::EnqueueViewportTask(ut::UniquePtr< ut::BaseTask<void> > task)
{
	if (!viewport_tasks.Add(ut::Move(task)))
	{
		throw ut::Error(ut::error::out_of_memory);
	}
}

// Enqueue a ResizeViewport() member function call and wait for completion
//    @param id - id of the viewport whose render display must be resized.
//    @param w - new width of render display in pixels.
//    @param h - new height of render display in pixels.
void ViewportManager::EnqueueResize(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h)
{
	// lock both current thread and render thread so that nothing could be rendered
	// while UI canvas is being resized, otherwise UI can damage render display
	// while resizing
	ut::SyncPoint::Lock lock = sync_point.AcquireLock();

	// check if viewport with provided id is present in the cache
	ut::Optional<size_t> viewport_array_id = FindViewport(id);
	if(!viewport_array_id)
	{
		return;
	}

	// manually resize UI area
	ui::PlatformViewport& ui_widget = viewports[viewport_array_id.Get()].ui_widget;
	ui_widget.ResizeCanvas();

	// enqueue resize task, display will be resized in render
	// thread right after the @lock will be released
	auto resize_proc = ut::MemberFunction<ViewportManager, ResizeFunction>(this, &ViewportManager::ResizeViewport);
	EnqueueViewportTask(ut::MakeUnique< ut::Task<ResizeFunction> >(ut::Move(resize_proc), id, w, h));
}

// Enqueue a CloseViewport() member function call and wait for completion
// in the synchronization point.
//    @param id - id of the viewport to be closed.
void ViewportManager::EnqueueClose(ui::Viewport::Id id)
{
	ut::SyncPoint::Lock lock = sync_point.AcquireLock();
	auto close_proc = ut::MemberFunction<ViewportManager, CloseFunction>(this, &ViewportManager::CloseViewport);
	EnqueueViewportTask(ut::MakeUnique< ut::Task<CloseFunction> >(ut::Move(close_proc), id));
}

// Searches for a viewport container associated with provided viewport id.
//    @return - id of the container in the @viewports array.
ut::Optional<size_t> ViewportManager::FindViewport(ui::Viewport::Id id)
{
	const size_t count = viewports.GetNum();
	for (size_t i = count; i--;)
	{
		if (viewports[i].ui_widget.GetId() == id)
		{
			return i;
		}
	}
	return ut::Optional<size_t>();
}

// Proxy constructor.
ViewportManager::Proxy::Proxy(ui::PlatformViewport& in_ui_viewport,
                              Display in_display,
                              RenderPass in_quad_pass,
                              PipelineState in_pipeline_state,
                              PipelineState in_pipeline_rgb2srgb,
                              PipelineState in_pipeline_srgb2rgb,
                              ut::Array<Framebuffer> in_framebuffers) : ui_widget(in_ui_viewport)
                                                                      , display(ut::Move(in_display))
                                                                      , quad_pass(ut::Move(in_quad_pass))
                                                                      , pipeline_state(ut::Move(in_pipeline_state))
                                                                      , pipeline_rgb2srgb(ut::Move(in_pipeline_rgb2srgb))
                                                                      , pipeline_srgb2rgb(ut::Move(in_pipeline_srgb2rgb))
                                                                      , framebuffers(ut::Move(in_framebuffers))
{}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
