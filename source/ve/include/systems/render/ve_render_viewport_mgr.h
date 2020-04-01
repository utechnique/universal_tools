//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_system.h"
#include "ve_render_api.h"
#include "systems/ui/ve_ui.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ViewportManager is a helper class processing events of UI
// viewports. It can map contents of the render::Display into ui::Viewport,
// can resize/close displays, etc.
class ViewportManager
{
public:
	// Construcot. Render thread is needed to operate with
	// displays that are associated with viewports.
	ViewportManager(ut::SharedPtr<Device::Thread> in_render_thread);

protected:
	// Initializes internal array of display/viewport pairs with one
	// provided by UI frontend.
	//    @param frontend - reference to ui::Frontend object.
	void OpenViewports(ui::Frontend& frontend);

	// Executes all viewport tasks in the queue.
	void ExecuteViewportTasks();

	// Searches for a display associated with provided viewport id.
	ut::Optional<render::Display&> FindDisplay(ui::Viewport::Id id);

	// Array of viewport/display pairs.
	ut::Array< ut::Container<ui::DesktopViewport&, Display> > viewports;

    // mutex to protect viewports
    ut::Mutex viewport_guard;

private:
    // Resizes a display associated with provided viewport.
	//    @param id - id of the viewport whose render display must be resized.
	//    @param w - new width of render display in pixels.
	//    @param h - new height of render display in pixels.
	void ResizeViewport(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h);

	// Removes viewport from internal map and enqueues display for deletion in
	// render thread.
	//    @param id - id of the viewport to be closed.
	void CloseViewport(ui::Viewport::Id id);

	// Sends render thread a request to delete provided display.
	//    @param id - unique pointer to the display to be deleted.
	void DeleteDisplay(ut::UniquePtr<render::Display> display);

    // Adds a new task to the queue. This task will be executed
    // in the next ProcessViewportTasks() call.
    void AddViewportTask(ut::UniquePtr< ut::BaseTask<void> > task);

    // Enqueues a resize task.
    void AddResizeTask(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h);

    // Enqueues a task to delete a display in render thread.
    //   @param display - unique pointer to the display to be deleted.
    void AddDeleteDisplayTask(ut::UniquePtr<render::Display> display);

	// Render thread is needed to operate with render displays
	// associated with managed viewports.
	ut::SharedPtr<Device::Thread> render_thread;

    // task can be added by calling AddViewportTask() and is executed
    // in ExecuteViewportTasks();
    ut::Array< ut::UniquePtr< ut::BaseTask<void> > > viewport_tasks;

    // mutex to protect tasks
    ut::Mutex viewport_task_guard;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
