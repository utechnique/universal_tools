//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui.h"
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ViewportManager is a helper class processing events of UI
// viewports associated with rendering. It manages render resources helping
// display to present its contents.
class ViewportManager
{
public:
	// Construcot.
	ViewportManager(ut::SharedPtr<Device::Thread> in_render_thread);

	// Destructor.
	~ViewportManager();

	// Move constructor.
	ViewportManager(ViewportManager&&);

	// Copying is prohibited.
	ViewportManager(const ViewportManager&) = delete;

	// Initializes internal array of viewport containers.
	//    @param frontend - reference to ui::Frontend object.
	void OpenViewports(ui::Frontend& frontend);

protected:
	// Resources needed to render textured quads directly to the backbuffer.
	struct Proxy
	{
		// Constructor.
		Proxy(ui::PlatformViewport& in_ui_viewport,
		      Display in_display,
		      RenderPass in_quad_pass,
		      PipelineState in_pipeline_state,
		      PipelineState in_pipeline_state_no_alpha,
		      PipelineState in_pipeline_rgb2srgb,
		      PipelineState in_pipeline_rgb2srgb_no_alpha,
		      PipelineState in_pipeline_srgb2rgb,
		      PipelineState in_pipeline_srgb2rgb_no_alpha,
		      ut::Array<Framebuffer> in_framebuffers);

		// ui viewport widget
		ui::PlatformViewport& ui_widget;

		// render display
		Display display;

		// render pass rendering textured quads
		RenderPass quad_pass;
		PipelineState pipeline_state, pipeline_state_no_alpha;
		PipelineState pipeline_rgb2srgb, pipeline_rgb2srgb_no_alpha;
		PipelineState pipeline_srgb2rgb, pipeline_srgb2rgb_no_alpha;

		// framebuffers for a swapchain
		ut::Array<Framebuffer> framebuffers;
	};

	// Synchronizes all viewport events. Must be called after previous frame is
	// finished and before the next one starts.
	void SyncViewportEvents();

	// Executes pending viewport tasks (resize, close, etc.)
	void ExecuteViewportTasks();

	// Returns 'true' if at least one viewport task is waiting to be executed.
	bool HasPendingViewportTasks();

	// Changes the number of buffers in a swap chain to the given value.
	void SetSwapBufferCount(ut::uint32 swap_buffer_count);

	// Enables or disables vertical synchronization.
	void SetVerticalSynchronization(bool status);

	// Array of viewports
	ut::Array<Proxy> viewports;

	// shaders rendering a quad to the backbuffer
	ut::Optional<Shader&> display_quad_vs, display_quad_ps;

	// pixel shaders drawing a quad to the backbuffer
	// with color space conversion.
	ut::Optional<Shader&> display_quad_rgb2srgb_ps, display_quad_srgb2rgb_ps;

private:
	// Viewport tasks are executed once per frame in a special synchronization
	// member function - SyncViewportEvents(). Thus all task issuers must wait
	// until the task is finished.
	typedef ut::UniquePtr< ut::BaseTask<void> > ViewportTask;

	// Creates a new display and all associated render resources for the
	// provided viewport.
	//    @param device - reference to the render device.
	//    @param viewport - reference to the viewport.
	//    @return - container with all render resources, or error if failed.
	ut::Result<Proxy, ut::Error> CreateDisplay(Device& device,
	                                           ui::PlatformViewport& viewport);

    // Resizes a display associated with provided viewport.
	//    @param id - id of the viewport whose render display must be resized.
	//    @param w - new width of render display in pixels.
	//    @param h - new height of render display in pixels.
	typedef void(ResizeFunction)(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h);
	ResizeFunction ResizeViewport;

	// Removes viewport from internal map and destroys render resources.
	//    @param id - id of the viewport to be closed.
	typedef void(CloseFunction)(ui::Viewport::Id id);
	CloseFunction CloseViewport;

	// Enqueues a task and waits for completion.
	void EnqueueViewportTask(ut::UniquePtr< ut::BaseTask<void> > task);

	// Enqueue a ResizeViewport() member function call and wait for completion
	//    @param id - id of the viewport whose render display must be resized.
	//    @param w - new width of render display in pixels.
	//    @param h - new height of render display in pixels.
	void EnqueueResize(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h);

	// Enqueue a CloseViewport() member function call and wait for completion
	// in the synchronization point.
	//    @param id - id of the viewport to be closed.
	void EnqueueClose(ui::Viewport::Id id);

	// Searches for a viewport container associated with provided viewport id.
	//    @return - id of the container in the @viewports array.
	ut::Optional<size_t> FindViewport(ui::Viewport::Id id);

	// Render thread is needed to operate with render displays
	// associated with managed viewports.
	ut::SharedPtr<Device::Thread> render_thread;

	// array of viewport tasks, only two functions are allowed to interact with
    // it in a safe maner: EnqueueViewportTask() to add task, and
	// SyncViewportEvents() to execute all tasks and clear the array
	ut::Array<ViewportTask> viewport_tasks;

	// synchronization point to strictly synchronize viewport events
	ut::SyncPoint sync_point;

	// the number of buffers in a swap chain
	ut::uint32 swap_buffer_count = 2;

	// vertical synchronization is either enabled for all viewports or disabled
	// for all viewports
	bool vertical_synchronization = true;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
