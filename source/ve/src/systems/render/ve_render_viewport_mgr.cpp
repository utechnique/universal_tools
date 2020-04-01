//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_viewport_mgr.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Construcot. Render thread is needed to operate with
// displays that are associated with viewports.
ViewportManager::ViewportManager(ut::SharedPtr<Device::Thread> rt) : render_thread(ut::Move(rt))
{
	UT_ASSERT(render_thread);
}

// Initializes internal array of display/viewport pairs with one
// provided by UI frontend.
//    @param frontend - reference to ui::Frontend object.
void ViewportManager::OpenViewports(ui::Frontend& frontend)
{
	ut::ScopeLock lock(viewport_guard);

	ut::Array< ut::UniquePtr<ui::Viewport> >::Iterator start = frontend.BeginViewports();
	ut::Array< ut::UniquePtr<ui::Viewport> >::Iterator end = frontend.EndViewports();

	for (ut::Array< ut::UniquePtr<ui::Viewport> >::Iterator iterator = start; iterator != end; iterator++)
	{
		// convert iterator to the reference to desktop viewport
		ut::UniquePtr<ui::Viewport>& viewport_ptr = *iterator;
		ui::DesktopViewport& viewport = static_cast<ui::DesktopViewport&>(viewport_ptr.GetRef());

		// create display for the viewport in the render thread
		ut::Result<render::Display, ut::Error> display_result = ut::MakeError(ut::error::empty);
		render_thread->Enqueue([&](render::Device& device) { display_result = device.CreateDisplay(viewport); });
		if (!display_result)
		{
			throw ut::Error(display_result.MoveAlt());
		}

		// connect signals
		viewport.ConnectCloseSignalSlot([this](ui::Viewport::Id id) { this->CloseViewport(id); });
		viewport.ConnectResizeSignalSlot([this](ui::Viewport::Id id, ut::uint32 w, ut::uint32 h) { this->AddResizeTask(id, w, h); });

		// newly created display is added to the map and can be
		// used to display rendered images to user
		if (!viewports.Add(ut::Container<ui::DesktopViewport&, Display>(viewport, display_result.MoveResult())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
}

// Executes all viewport tasks in the queue.
void ViewportManager::ExecuteViewportTasks()
{
    // copy tasks to the temp buffer
    viewport_task_guard.Lock();
    ut::Array< ut::UniquePtr< ut::BaseTask<void> > > task_buffer = ut::Move(viewport_tasks);
    viewport_task_guard.Unlock();

    // execute tasks
    ut::ScopeLock lock(viewport_guard);
    const size_t task_count = task_buffer.GetNum();
    for (size_t i = 0; i < task_count; i++)
    {
        task_buffer[i]->Execute();
    }
}

// Resizes a display associated with provided viewport.
//    @param id - id of the viewport whose render display must be resized.
//    @param w - new width of render display in pixels.
//    @param h - new height of render display in pixels.
void ViewportManager::ResizeViewport(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h)
{
	// lock isn't needed here because resize() can happen only in the fltk
	// thread and viewport is guaranteed to be alive there (fltk owns viewport)
	ut::Optional<render::Display&> display = FindDisplay(id);
	if (display)
	{
		render_thread->Enqueue([&](render::Device& device) { device.ResizeDisplay(display.Get(), w, h); });
	}
}

// Removes viewport from internal map and enqueues display for deletion in
// render thread.
//    @param id - id of the viewport to be closed.
void ViewportManager::CloseViewport(ui::Viewport::Id id)
{
    ut::UniquePtr<render::Display> display_to_delete;

	viewport_guard.Lock();
	const size_t count = viewports.GetNum();
	for (size_t i = count; i--;)
	{
		ui::DesktopViewport& viewport = viewports[i].Get<ui::DesktopViewport&>();
		if (viewport.GetId() == id)
		{
            display_to_delete = ut::MakeUnique<render::Display>(ut::Move(viewports[i].Get<render::Display>()));
            viewports.Remove(i);
            break;
		}
	}
	viewport_guard.Unlock();

    if (display_to_delete)
    {
        AddDeleteDisplayTask(ut::Move(display_to_delete));
    }
}

// Sends render thread a request to delete provided display.
//    @param id - unique pointer to the display to be deleted.
void ViewportManager::DeleteDisplay(ut::UniquePtr<render::Display> display)
{
    render_thread->Enqueue([&](render::Device&) { display.Delete(); });
}

// Searches for a display associated with provided viewport id.
ut::Optional<render::Display&> ViewportManager::FindDisplay(ui::Viewport::Id id)
{
	const size_t count = viewports.GetNum();
	for (size_t i = count; i--;)
	{
		ui::DesktopViewport& viewport = viewports[i].Get<ui::DesktopViewport&>();
		if (viewport.GetId() == id)
		{
			return viewports[i].Get<render::Display>();
		}
	}
	return ut::Optional<render::Display&>();
}

// Adds a new task to the queue. This task will be executed
// in the next ProcessViewportTasks() call.
void ViewportManager::AddViewportTask(ut::UniquePtr< ut::BaseTask<void> > task)
{
    ut::ScopeLock lock(viewport_task_guard);
    if (!viewport_tasks.Add(ut::Move(task)))
    {
        throw ut::Error(ut::error::out_of_memory);
    }
}

// Enqueues a resize task.
void ViewportManager::AddResizeTask(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h)
{
    auto resize_proc = ut::MemberFunction<ViewportManager, void(ui::Viewport::Id, ut::uint32, ut::uint32)>(this, &ViewportManager::ResizeViewport);

    AddViewportTask(ut::MakeUnique< ut::Task<void(ui::Viewport::Id, ut::uint32, ut::uint32)> >(ut::Move(resize_proc), id, w, h));
}

// Enqueues a task to delete a display in render thread.
//   @param display - unique pointer to the display to be deleted.
void ViewportManager::AddDeleteDisplayTask(ut::UniquePtr<render::Display> display)
{
    auto delete_proc = ut::MemberFunction<ViewportManager, void(ut::UniquePtr<render::Display>)>(this, &ViewportManager::DeleteDisplay);
    AddViewportTask(ut::MakeUnique< ut::Task<void(ut::UniquePtr<render::Display>)> >(ut::Move(delete_proc), ut::Move(display)));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
