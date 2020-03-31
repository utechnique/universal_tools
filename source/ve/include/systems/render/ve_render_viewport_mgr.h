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

	// Maps contents of the render displays to associated ui viewports.
	void UpdateViewports();

	// Resizes a display associated with provided viewport.
	//    @param id - id of the viewport whose render display must be resized.
	//    @param w - new width of render display in pixels.
	//    @param h - new height of render display in pixels.
	void ResizeViewport(ui::Viewport::Id id, ut::uint32 w, ut::uint32 h);

	// Deletes render display associated with provided viewport and removes
	// this viewport from internal map.
	//    @param id - id of the viewport to be closed.
	void CloseViewport(ui::Viewport::Id id);

	// Searches for a display associated with provided viewport id.
	ut::Optional<render::Display&> FindDisplay(ui::Viewport::Id id);
	
	// Mutex to synchronize viewports.
	ut::Mutex viewport_guard;

	// Array of viewport/display pairs.
	ut::Array< ut::Container<ui::DesktopViewport&, Display> > viewports;

	// Render thread is needed to operate with render displays
	// associated with managed viewports.
	ut::SharedPtr<Device::Thread> render_thread;

};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
