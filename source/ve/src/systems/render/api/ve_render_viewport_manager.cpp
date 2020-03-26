//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_viewport_manager.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
ViewportManager::ViewportManager() = default;

// Move constructor.
ViewportManager::ViewportManager(ViewportManager&&) noexcept = default;

// Move operator.
ViewportManager& ViewportManager::operator =(ViewportManager&&) noexcept = default;

// Creates a new display using provided viewport widget and adds it to the map.
//    @param viewport - reference to UI viewport containing rendering area.
ut::Optional<ut::Error> ViewportManager::AddViewport(ui::Viewport& viewport)
{
	// extract unique id of the viewport.
	const ui::Viewport::Id id = viewport.GetId();

	// check if display with such id already exists
	if (displays.Find(id))
	{
		return ut::Error(ut::error::already_exists);
	}

	// create platform-specific display
	ut::Result<Display, ut::Error> platform_display = CreateDisplay(viewport);
	if (!platform_display)
	{
		return platform_display.MoveAlt();
	}

	// create final display object
	if (!displays.Insert(id, platform_display.MoveResult()))
	{
		return ut::Error(ut::error::out_of_memory);
	}

	// success
	return ut::Optional<ut::Error>();
}

// Resizes buffers associated with rendering area inside a UI viewport.
//    @param id - unique id of the viewport associated with rendering display.
//    @param width - new width of the display in pixels.
//    @param width - new height of the display in pixels.
//    @return - optional ut::Error if failed.
ut::Optional<ut::Error> ViewportManager::ResizeViewport(ui::Viewport::Id id,
                                                        ut::uint32 width,
                                                        ut::uint32 height)
{
	// check if display with such id already exists
	ut::Optional<Display&> display_result = displays.Find(id);
	if (!display_result)
	{
		return ut::Error(ut::error::not_found);
	}

	// platform specific code to resize a viewport
	return ResizeDisplay(display_result.Get(), width, height);
}

// Removes specified display from the map.
//    @param id - unique id of the viewport associated with rendering display.
void ViewportManager::RemoveViewport(ui::Viewport::Id id)
{
	displays.Remove(id);
}

// Returns a reference to the display associated with specified viewport's id
// or nothing if this id wasn't found in the map.
ut::Optional<Display&> ViewportManager::GetDisplay(ui::Viewport::Id id)
{
	return displays.Find(id);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//