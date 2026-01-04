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
{
	UT_ASSERT(render_thread);
}

// Destructor.
ViewportManager::~ViewportManager()
{
	// reset viewport signals, otherwise viewports will signal
	// to the non-existent viewport manager, causing access violation
	for (ut::uint32 i = 0; i < viewports.Count(); i++)
	{
		viewports[i].ui_widget.ResetSignals();
	}

	// close viewports manually
	for (ut::uint32 i = 0; i < viewports.Count(); i++)
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
		render_thread->Enqueue([&](Device& device) { result = CreateDisplay(viewport); });
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
	const size_t task_count = viewport_tasks.Count();
	for (size_t i = 0; i < task_count; i++)
	{
		viewport_tasks[i]->Execute();
	}
	viewport_tasks.Reset();
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
		ut::Result<Proxy, ut::Error> vp_container_result = CreateDisplay(ui_widget);
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
	const size_t count = viewports.Count();
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
                              ut::Array<PipelineState> in_pipeline,
                              ut::Array<Framebuffer> in_framebuffers) : ui_widget(in_ui_viewport)
                                                                      , display(ut::Move(in_display))
                                                                      , quad_pass(ut::Move(in_quad_pass))
                                                                      , pipeline(ut::Move(in_pipeline))
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
