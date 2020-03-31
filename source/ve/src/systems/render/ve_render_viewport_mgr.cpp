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
		viewport.ConnectCloseSignalSlot([&](ui::Viewport::Id id) { CloseViewport(id); });
		viewport.ConnectResizeSignalSlot([&](ui::Viewport::Id id, ut::uint32 w, ut::uint32 h) { ResizeViewport(id, w, h); });

		// newly created display is added to the map and can be
		// used to display rendered images to user
		if (!viewports.Add(ut::Container<ui::DesktopViewport&, Display>(viewport, display_result.MoveResult())))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}
}

// Maps contents of the render displays to associated ui viewports.
void ViewportManager::UpdateViewports()
{
	ut::ScopeLock viewport_lock(viewport_guard);
	const size_t count = viewports.GetNum();
	for (size_t i = 0; i < count; i++)
	{
		render::Display& display = viewports[i].Get<render::Display>();
		display.Present(false);
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

// Deletes render display associated with provided viewport and removes
// this viewport from internal map.
//    @param id - id of the viewport to be closed.
void ViewportManager::CloseViewport(ui::Viewport::Id id)
{
	ut::ScopeLock lock(viewport_guard);
	const size_t count = viewports.GetNum();
	for (size_t i = count; i--;)
	{
		ui::DesktopViewport& viewport = viewports[i].Get<ui::DesktopViewport&>();
		viewports.Remove(i);
	}
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

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
