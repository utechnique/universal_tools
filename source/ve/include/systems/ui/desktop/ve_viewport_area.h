//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
#include "ve_id_generator.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::ViewportContainer contains viewport, additional tools to interact
// with it and a frame.
class ViewportBox : public Fl_Group
{
public:
	// Constructor.
	ViewportBox(const Settings& settings,
	            Viewport::Id id,
	            ut::uint32 x,
	            ut::uint32 y,
	            ut::uint32 w,
	            ut::uint32 h);

	// Returns a reference to the viewport widget.
	DesktopViewport& GetViewport();

private:
	ut::UniquePtr<DesktopViewport> viewport;
};

//----------------------------------------------------------------------------//
// ve::ui::ViewportArea contains viewports arranged in a special layout.
class ViewportArea : public Fl_Tile
{
	static constexpr ut::uint32 skMaxViewports = 4;
public:
	// Constructor.
	ViewportArea(const Settings& settings,
	             ut::uint32 x,
	             ut::uint32 y,
	             ut::uint32 w,
	             ut::uint32 h);

	// Generates and returns an array of references to the viewports.
	ut::Array< ut::Ref<Viewport> > GetViewports();

private:

	// box to set resizing range of viewports
	ut::UniquePtr<Fl_Box> border_box;

	// viewports are placed in special 'box' widgets
	ut::UniquePtr<ViewportBox> viewport_boxes[skMaxViewports];

	// Id-generator is used to generate unique identifiers for viewports.
	IdGenerator<Viewport::Id> id_generator;

	// boundary for viewport resizing
	static const ut::uint32 skResizeBorder;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
