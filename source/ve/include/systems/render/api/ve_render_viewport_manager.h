//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/render/api/ve_render_display.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ViewportManager is a helper class acting as an intermediary
// between the user interface and rendering api.
class ViewportManager
{
public:
	// Constructor.
	ViewportManager();

	// Move constructor.
	ViewportManager(ViewportManager&&) noexcept;

	// Move operator.
	ViewportManager& operator =(ViewportManager&&) noexcept;

	// Copying is prohibited.
	ViewportManager(const ViewportManager&) = delete;
	ViewportManager& operator =(const ViewportManager&) = delete;

	// Creates platform-specific representation of the rendering area inside a UI viewport.
	//    @param viewport - reference to UI viewport containing rendering area.
	//    @return - new display object or error if failed.
	virtual ut::Result<Display, ut::Error> CreateDisplay(ui::Viewport& viewport) = 0;

	// Resizes buffers associated with rendering area inside a UI viewport.
	//    @param display - reference to display object.
	//    @param width - new width of the display in pixels.
	//    @param width - new height of the display in pixels.
	//    @return - optional ut::Error if failed.
	virtual ut::Optional<ut::Error> ResizeDisplay(Display& display,
	                                              ut::uint32 width,
	                                              ut::uint32 height) = 0;

	// Creates a new display using provided viewport widget and adds it to the map.
	//    @param viewport - reference to UI viewport containing rendering area.
	ut::Optional<ut::Error> AddViewport(ui::Viewport& viewport);

	// Resizes buffers associated with rendering area inside a UI viewport.
	//    @param id - unique id of the viewport associated with rendering display.
	//    @param width - new width of the display in pixels.
	//    @param width - new height of the display in pixels.
	//    @return - optional ut::Error if failed.
	ut::Optional<ut::Error> ResizeViewport(ui::Viewport::Id id,
	                                       ut::uint32 width,
	                                       ut::uint32 height);

	// Removes specified display from the map.
	//    @param id - unique id of the viewport associated with rendering display.
	void RemoveViewport(ui::Viewport::Id id);

	// Returns a reference to the display associated with specified viewport's id
	// or nothing if this id wasn't found in the map.
	ut::Optional<Display&> GetDisplay(ui::Viewport::Id id);

private:
	ut::Map<ui::Viewport::Id, Display> displays;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//